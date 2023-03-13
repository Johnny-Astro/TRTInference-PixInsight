#include "TRTInferenceParameters.h"

namespace pcl
{

TRTInferenceTileOverlap* TheTRTInferenceTileOverlapParameter = nullptr;
TRTInferenceKeepOutputDimension* TheTRTInferenceKeepOutputDimensionParameter = nullptr;

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

TRTInferenceKeepOutputDimension::TRTInferenceKeepOutputDimension(MetaProcess* P) : MetaBoolean(P)
{
    TheTRTInferenceKeepOutputDimensionParameter = this;
}

IsoString TRTInferenceKeepOutputDimension::Id() const
{
    return "keepOutputDimension";
}

bool TRTInferenceKeepOutputDimension::DefaultValue() const
{
    return false;
}

}	// namespace pcl