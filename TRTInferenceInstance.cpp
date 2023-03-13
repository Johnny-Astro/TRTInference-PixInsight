#include <pcl/AutoViewLock.h>
#include <pcl/Console.h>
#include <pcl/File.h>
#include <pcl/IntegerResample.h>
#include <pcl/Resample.h>
#include <pcl/StandardStatus.h>
#include <pcl/View.h>

#include "TRTInferenceInstance.h"
#include "TRTInferenceParameters.h"
#include "TRTInferenceProcess.h"

namespace pcl
{

void TRTLogger::log(Severity severity, const char* msg) noexcept
{
    if (severity > Severity::kWARNING)
        return;
    Console console;
    if (severity == Severity::kWARNING)
        console.WarningLn(String("[TensorRT] WARNING: ") + msg);
    else
        console.CriticalLn(String("[TensorRT] ERROR: ") + msg);
}

TRTEngine::TRTEngine(String enginePath, const char* inputBlobName, const char* outputBlobName)
{
    strcpy(m_inputBlobName, inputBlobName);
    strcpy(m_outputBlobName, outputBlobName);

    File file(enginePath, FileMode::Read);
    if (!file.IsOpen())
        throw Error("Unable to open TensorRT engine file " + enginePath);
    auto size = file.Size();
    ByteArray buffer(size);
    file.Read(buffer.Begin(), size);
    file.Close();

    auto runtime = std::unique_ptr<nvinfer1::IRuntime>(nvinfer1::createInferRuntime(m_logger));
    if (!runtime)
        throw Error("Failed to create TensorRT runtime.");

    cudaSetDevice(0);

    m_engine = std::unique_ptr<nvinfer1::ICudaEngine>(runtime->deserializeCudaEngine(buffer.Begin(), size));
    if (!m_engine)
        throw Error("Failed to deserialize TensorRT engine from engine file " + enginePath);

    auto dims = m_engine->getTensorShape(m_inputBlobName);
    if ((dims.d[0] == -1) || (m_engine->getTensorIOMode(m_inputBlobName) != nvinfer1::TensorIOMode::kINPUT))
        throw Error("Input blob " + String(m_inputBlobName) + " not found.");
    if (dims.nbDims != 4)
        throw Error("Input blob " + String(m_inputBlobName) + " is not 4-dimension.");
    if (dims.d[1] != 3)
        throw Error("Input blob " + String(m_inputBlobName) + " does not have 3 channels.");
    m_inputTileH = dims.d[2];
    m_inputTileW = dims.d[3];

    dims = m_engine->getTensorShape(m_outputBlobName);
    if ((dims.d[0] == -1) || (m_engine->getTensorIOMode(m_outputBlobName) != nvinfer1::TensorIOMode::kOUTPUT))
        throw Error("Output blob " + String(m_outputBlobName)+" not found.");
    if (dims.nbDims != 4)
        throw Error("Output blob " + String(m_outputBlobName)+" is not 4-dimension.");
    if (dims.d[1] != 3)
        throw Error("Output blob " + String(m_outputBlobName)+" does not have 3 channels.");
    m_outputTileH = dims.d[2];
    m_outputTileW = dims.d[3];

    if (((m_outputTileW % m_inputTileW) != 0) || ((m_outputTileH % m_inputTileH) != 0))
        throw Error("Shape of output blob " + String(m_outputBlobName)+" is not multiple of input blob " + m_inputBlobName);

    auto datatype = m_engine->getTensorDataType(m_inputBlobName);
    if (datatype != nvinfer1::DataType::kFLOAT)
        throw Error("Input blob " + String(m_inputBlobName)+" is not 32-bit float.");
    datatype = m_engine->getTensorDataType(m_outputBlobName);
    if (datatype != nvinfer1::DataType::kFLOAT)
        throw Error("Output blob " + String(m_outputBlobName)+" is not 32-bit float.");

    m_context = std::unique_ptr<nvinfer1::IExecutionContext>(m_engine->createExecutionContext());
    if (!m_context)
        throw Error("Failed to create TensorRT execution context.");

    if (cudaStreamCreate(&m_cudaStream) != cudaSuccess)
        throw Error("Failed to create CUDA stream.");
}

TRTEngine::~TRTEngine()
{
    cudaStreamDestroy(m_cudaStream);
    m_context.reset();
    m_engine.reset();
}

void TRTEngine::runInferenceTile(const FImage& input, const Point tilePos, FImage& output, FImage& mask)
{
    auto dims = m_engine->getTensorShape(m_inputBlobName);
    dims.d[0] = 1;
    if (!m_context->setInputShape(m_inputBlobName, dims))
        throw Error("Failed to set input shape.");
    m_inputBuffer.hostBuffer.resize(dims);
    m_inputBuffer.deviceBuffer.resize(dims);
    dims = m_context->getTensorShape(m_outputBlobName);
    m_outputBuffer.hostBuffer.resize(dims);
    m_outputBuffer.deviceBuffer.resize(dims);

    // Get tile
    float* p = static_cast<float*>(m_inputBuffer.hostBuffer.data());
    for (int c = 0; c < 3; c++)
        for (int y = 0; y < m_inputTileH; y++)
        {
            int y0 = y + tilePos.y;
            if (y0 >= input.Height())
                y0 = input.Height() - 1;
            for (int x = 0; x < m_inputTileW; x++)
            {
                int x0 = x + tilePos.x;
                if (x0 >= input.Width())
                    x0 = input.Width() - 1;
                *p++ = input.Pixel(x0, y0, (input.NumberOfChannels() == 3) ? c : 0);
            }
        }

    // Copy from CPU to GPU
    auto ret = cudaMemcpyAsync(m_inputBuffer.deviceBuffer.data(), m_inputBuffer.hostBuffer.data(), m_inputBuffer.hostBuffer.nbBytes(), cudaMemcpyHostToDevice, m_cudaStream);
    if (ret != cudaSuccess)
        throw Error("Failed to send image tile to GPU.");

    // Run inference
    if (!m_context->setTensorAddress(m_inputBlobName, m_inputBuffer.deviceBuffer.data()))
        throw Error("Failed to set input tensors.");
    if (!m_context->setTensorAddress(m_outputBlobName, m_outputBuffer.deviceBuffer.data()))
        throw Error("Failed to set output tensors.");
    if (!m_context->enqueueV3(m_cudaStream))
        throw Error("Failed to run inference on image tile.");

    // Copy from GPU to CPU
    ret = cudaMemcpyAsync(m_outputBuffer.hostBuffer.data(), m_outputBuffer.deviceBuffer.data(), m_outputBuffer.deviceBuffer.nbBytes(), cudaMemcpyDeviceToHost, m_cudaStream);
    if (ret != cudaSuccess)
        throw Error("Failed to receive image tile from GPU.");

    // Wait for CUDA stream
    ret = cudaStreamSynchronize(m_cudaStream);
    if (ret != cudaSuccess)
        throw Error("Failed to synchronize CUDA stream.");

    // Copy tile to output
    Point outputTilePos(tilePos.x * (m_outputTileW / m_inputTileW), tilePos.y * (m_outputTileH / m_inputTileH));
    p = static_cast<float*>(m_outputBuffer.hostBuffer.data());
    for (int c = 0; c < 3; c++)
        for (int y = 0; y < m_outputTileH; y++)
        {
            int y0 = y + outputTilePos.y;
            for (int x = 0; x < m_outputTileW; x++)
            {
                int x0 = x + outputTilePos.x;
                if ((x0 >= output.Width()) || (y0 >= output.Height()))
                {
                    p++;
                    continue;
                }
                float r = 0.5f - Abs(x - m_outputTileW / 2) * 1.0f / m_outputTileW;
                r *= 0.5f - Abs(y - m_outputTileH / 2) * 1.0f / m_outputTileH;
                if (r == 0.0f)
                    r = 0.001f;
                output.Pixel(x0, y0, c) += *p++ * r;
                mask.Pixel(x0, y0, c) += r;
            }
        }
}

TRTInferenceInstance::TRTInferenceInstance(const MetaProcess* m)
    : ProcessImplementation(m)
    , p_tileOverlap(TheTRTInferenceTileOverlapParameter->DefaultValue())
{
}

TRTInferenceInstance::TRTInferenceInstance(const TRTInferenceInstance& x)
    : ProcessImplementation(x)
{
    Assign(x);
}

void TRTInferenceInstance::Assign(const ProcessImplementation& p)
{
    const TRTInferenceInstance* x = dynamic_cast<const TRTInferenceInstance*>(&p);
    if (x != nullptr)
    {
        p_trtEngine = x->p_trtEngine;
        p_tileOverlap = x->p_tileOverlap;
    }
}

bool TRTInferenceInstance::IsHistoryUpdater(const View& view) const
{
    return true;
}

bool TRTInferenceInstance::CanExecuteOn(const View& view, String& whyNot) const
{
    if (view.Image().IsComplexSample())
    {
        whyNot = String(TRTInferenceProcess::MODULE_NAME) + " cannot be executed on complex images.";
        return false;
    }
    else if (view.Image().BitsPerSample() == 64)
    {
        whyNot = String(TRTInferenceProcess::MODULE_NAME) + " cannot be executed on 64-bit images.";
        return false;
    }
    else if ((view.Image().NumberOfChannels() != 1) && (view.Image().NumberOfChannels() != 3))
    {
        whyNot = String(TRTInferenceProcess::MODULE_NAME) + " can only be executed on single-channle or RGB images";
        return false;
    }

    return true;
}

bool TRTInferenceInstance::ExecuteOn(View& view)
{
    String why;
    if (!CanExecuteOn(view, why))
        throw Error(why);

    AutoViewLock lock(view);

    StandardStatus status;
    Console console;

    console.EnableAbort();

    ImageVariant image = view.Image();
    image.SetStatusCallback(&status);

    TRTEngine trtEngine(p_trtEngine);
    int factorW = trtEngine.getOutputTileW() / trtEngine.getInputTileW();
    int factorH = trtEngine.getOutputTileH() / trtEngine.getInputTileH();

    ImageVariant imgToTRT;
    imgToTRT.CreateFloatImage();
    imgToTRT.AllocateImage(image.Width(), image.Height(), image.NumberOfChannels(), image.ColorSpace());
    imgToTRT.CopyImage(image);
    imgToTRT.SetStatusCallback(nullptr);

    ImageVariant imgFromTRT, mask;
    imgFromTRT.CreateFloatImage();
    imgFromTRT.AllocateImage(image.Width() * factorW, image.Height() * factorH, 3, ImageVariant::color_space::RGB);
    imgFromTRT.Zero();
    imgFromTRT.SetStatusCallback(nullptr);
    mask.CopyImage(imgFromTRT);
    mask.EnsureUniqueImage();
    mask.SetStatusCallback(nullptr);

    // Tile processing
    int tileStepX = trtEngine.getInputTileW() * (1.0f - p_tileOverlap);
    int tileStepY = trtEngine.getInputTileH() * (1.0f - p_tileOverlap);
    int n = (image.Width() + tileStepX - 1) / tileStepX;
    n *= (image.Height() + tileStepY - 1) / tileStepY;
    image.Status().Initialize("Running inference", n);
    for (int y = 0; y < image.Height(); y += tileStepY)
        for (int x = 0; x < image.Width(); x += tileStepX)
        {
            trtEngine.runInferenceTile(static_cast<const FImage&>(*imgToTRT), Point(x, y), static_cast<FImage&>(*imgFromTRT), static_cast<FImage&>(*mask));
            image.Status() += 1;
        }
    image.Status().Complete();

    imgFromTRT.Divide(mask);

    // Resample
    bool useIntegerResample = false;
    if ((factorW > 1) && (factorW == factorH))
    {
        IntegerResample ir(-factorW);
        ir >> imgFromTRT;
    }
    else if (factorW > 1)
    {
        BicubicFilterPixelInterpolation bf(factorW / 2, factorH / 2, CubicBSplineFilter());
        Resample r(bf, 1.0 / factorW, 1.0 / factorH);
        r >> imgFromTRT;
    }

    image.CopyImage(imgFromTRT);

    return true;
}

void* TRTInferenceInstance::LockParameter(const MetaParameter* p, size_type tableRow)
{
    if (p == TheTRTInferenceTileOverlapParameter)
        return &p_tileOverlap;
    return nullptr;
}

}	// namespace pcl