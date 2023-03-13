#ifndef __TRTInferenceInterface_h
#define __TRTInferenceInterface_h

#include <pcl/ProcessInterface.h>
#include <pcl/Sizer.h>
#include <pcl/ToolButton.h>
#include <pcl/NumericControl.h>

#include "TRTInferenceInstance.h"

namespace pcl {

class TRTInferenceInterface : public ProcessInterface
{
public:
    TRTInferenceInterface();
    virtual ~TRTInferenceInterface();

    IsoString Id() const override;
    MetaProcess* Process() const override;
    IsoString IconImageSVG() const override;
    InterfaceFeatures Features() const override;
    void ResetInstance() override;
    bool Launch(const MetaProcess&, const ProcessImplementation*, bool& dynamic, unsigned& /*flags*/) override;
    ProcessImplementation* NewProcess() const override;
    bool ValidateProcess(const ProcessImplementation&, pcl::String& whyNot) const override;
    bool RequiresInstanceValidation() const override;
    bool ImportProcess(const ProcessImplementation&) override;
    void SaveSettings() const override;

private:

    TRTInferenceInstance m_instance;

    struct GUIData
    {
        GUIData(TRTInferenceInterface&);

        VerticalSizer   Global_Sizer;

        Control         TRTEngine_Control;
            HorizontalSizer TRTEngine_Sizer;
                Label           TRTEngine_Label;
                Edit            TRTEngine_Edit;
                ToolButton      TRTEngine_ToolButton;

        Control         Inference_Control;
            VerticalSizer   Inference_Sizer;
                NumericControl  TileOverlap_NumericControl;
    };

    GUIData* GUI = nullptr;

    void UpdateControls();
    void __Click(Button& sender, bool checked);
    void __EditCompleted(Edit& sender);
    void __EditValueUpdated(NumericEdit& sender, double value);

    friend struct GUIData;
};

PCL_BEGIN_LOCAL
extern TRTInferenceInterface* TheTRTInferenceInterface;
PCL_END_LOCAL

}	// namespace pcl

#endif  // __TRTInferenceInterface_h
