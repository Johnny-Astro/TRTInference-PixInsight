#ifndef __TRTInferenceModule_h
#define __TRTInferenceModule_h

#include <pcl/MetaModule.h>

namespace pcl
{

class TRTInferenceModule : public MetaModule
{
public:
    TRTInferenceModule();

    const char* Version() const override;
    IsoString Name() const override;
    String Description() const override;
    String Company() const override;
    String Author() const override;
    String Copyright() const override;
    String TradeMarks() const override;
    String OriginalFileName() const override;
    void GetReleaseDate(int& year, int& month, int& day) const override;
};

}   // namespace pcl

#endif  // __TRTInferenceModule_h
