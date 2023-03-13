#ifndef __TRTInferenceInstance_h
#define __TRTInferenceInstance_h

#include <pcl/ProcessImplementation.h>
#include <NvInfer.h>
#include <buffers.h>

namespace pcl
{

class TRTLogger : public nvinfer1::ILogger
{
public:
    void log(Severity severity, const char* msg) noexcept override;
};

class TRTEngine
{
private:
    char m_inputBlobName[256];
    char m_outputBlobName[256];
    TRTLogger m_logger;
    std::unique_ptr<nvinfer1::ICudaEngine> m_engine;
    std::unique_ptr<nvinfer1::IExecutionContext> m_context;
    cudaStream_t m_cudaStream;
    int32_t m_inputTileW;
    int32_t m_inputTileH;
    int32_t m_outputTileW;
    int32_t m_outputTileH;
    samplesCommon::ManagedBuffer m_inputBuffer;
    samplesCommon::ManagedBuffer m_outputBuffer;

public:
    explicit TRTEngine(String enginePath, const char* inputBlobName = "input", const char* outputBlobName = "output");
    ~TRTEngine();

    int32_t getInputTileW() const
    {
        return m_inputTileW;
    }

    int32_t getInputTileH() const
    {
        return m_inputTileH;
    }

    int32_t getOutputTileW() const
    {
        return m_outputTileW;
    }

    int32_t getOutputTileH() const
    {
        return m_outputTileH;
    }

    void runInferenceTile(const pcl::FImage& input, const pcl::Point tilePos, pcl::FImage& output, pcl::FImage& mask);
};

class TRTInferenceInstance : public ProcessImplementation
{
public:
    TRTInferenceInstance(const MetaProcess*);
    TRTInferenceInstance(const TRTInferenceInstance&);

    void Assign(const ProcessImplementation&) override;
    bool IsHistoryUpdater(const View& v) const override;
    bool CanExecuteOn(const View&, String& whyNot) const override;
    bool ExecuteOn(View& view) override;
    void* LockParameter(const MetaParameter*, size_type tableRow) override;

private:
    String p_trtEngine;
    double p_tileOverlap;

    friend class TRTInferenceProcess;
    friend class TRTInferenceInterface;
};

}	// namespace pcl

#endif	// __TRTInferenceInstance_h