#include "TRTInferenceParameters.h"

namespace pcl
{

TRTInferenceTileOverlap* TheTRTInferenceTileOverlapParameter = nullptr;

TRTInferenceTileOverlap::TRTInferenceTileOverlap(MetaProcess* P) : MetaFloat(P)
{
    TheTRTInferenceTileOverlapParameter = this;
}

IsoString TRTInferenceTileOverlap::Id() const
{
    return "tileOverlap";
}

int TRTInferenceTileOverlap::Precision() const
{
    return 2;
}

double TRTInferenceTileOverlap::MinimumValue() const
{
    return 0.0;
}

double TRTInferenceTileOverlap::MaximumValue() const
{
    return 0.5;
}

double TRTInferenceTileOverlap::DefaultValue() const
{
    return 0.2;
}

}	// namespace pcl