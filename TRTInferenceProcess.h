#ifndef __TRTInferenceProcess_h
#define __TRTInferenceProcess_h

#include <pcl/MetaProcess.h>

namespace pcl
{

class TRTInferenceProcess : public MetaProcess
{
public:
    static constexpr const char* MODULE_ID = "TRTInference";
    static constexpr const char* MODULE_NAME = "TensorRT Inference Runner";
    static constexpr const char* MODULE_DESCRIPTION = "Run AI inference on the image using NVIDIA TensorRT engine";
    static constexpr uint32_t MODULE_VERSION = 0x100;

    TRTInferenceProcess();

    IsoString Id() const override;
    IsoString Category() const override;
    uint32 Version() const override;
    String Description() const override;
    IsoString IconImageSVG() const override;
    ProcessInterface* DefaultInterface() const override;
    ProcessImplementation* Create() const override;
    ProcessImplementation* Clone(const ProcessImplementation&) const override;
    bool NeedsValidation() const override;
    bool CanProcessCommandLines() const override;
};

PCL_BEGIN_LOCAL
extern TRTInferenceProcess* TheTRTInferenceProcess;
PCL_END_LOCAL

}	// namespace pcl

#endif	// __TRTInferenceProcess_h