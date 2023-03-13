#ifndef __TRTInferenceParameters_h
#define __TRTInferenceParameters_h

#include <pcl/MetaParameter.h>

namespace pcl
{

PCL_BEGIN_LOCAL

class TRTInferenceTileOverlap : public MetaFloat
{
public:
    TRTInferenceTileOverlap(MetaProcess*);

    IsoString Id() const override;
    int Precision() const override;
    double MinimumValue() const override;
    double MaximumValue() const override;
    double DefaultValue() const override;
};

extern TRTInferenceTileOverlap* TheTRTInferenceTileOverlapParameter;

PCL_END_LOCAL

}	// namespace pcl

#endif	// __TRTInferenceParameters_h
