#define MODULE_VERSION_MAJOR     1
#define MODULE_VERSION_MINOR     0
#define MODULE_VERSION_REVISION  0
#define MODULE_VERSION_BUILD     0
#define MODULE_VERSION_LANGUAGE  eng

#define MODULE_RELEASE_YEAR      2023
#define MODULE_RELEASE_MONTH     3
#define MODULE_RELEASE_DAY       10

#include "TRTInferenceModule.h"
#include "TRTInferenceProcess.h"
#include "TRTInferenceInterface.h"

namespace pcl
{

TRTInferenceModule::TRTInferenceModule()
{
}

const char* TRTInferenceModule::Version() const
{
    return PCL_MODULE_VERSION(MODULE_VERSION_MAJOR,
                              MODULE_VERSION_MINOR,
                              MODULE_VERSION_REVISION,
                              MODULE_VERSION_BUILD,
                              MODULE_VERSION_LANGUAGE);
}

IsoString TRTInferenceModule::Name() const
{
    return TRTInferenceProcess::MODULE_ID;
}

String TRTInferenceModule::Description() const
{
    return String("PixInsight ") + TRTInferenceProcess::MODULE_NAME + " Module";
}

String TRTInferenceModule::Company() const
{
    return "N/A";
}

String TRTInferenceModule::Author() const
{
    return "Johnny Qiu";
}

String TRTInferenceModule::Copyright() const
{
    return "Copyright (c) 2023 Johnny Qiu";
}

String TRTInferenceModule::TradeMarks() const
{
    return "JQ";
}

String TRTInferenceModule::OriginalFileName() const
{
#ifdef __PCL_LINUX
    return String(TRTInferenceProcess::MODULE_ID) + "-pxm.so";
#endif
#ifdef __PCL_FREEBSD
    return String(TRTInferenceProcess::MODULE_ID) + "-pxm.so";
#endif
#ifdef __PCL_MACOSX
    return String(TRTInferenceProcess::MODULE_ID) + "-pxm.dylib";
#endif
#ifdef __PCL_WINDOWS
    return String(TRTInferenceProcess::MODULE_ID) + "-pxm.dll";
#endif
}

void TRTInferenceModule::GetReleaseDate(int& year, int& month, int& day) const
{
    year = MODULE_RELEASE_YEAR;
    month = MODULE_RELEASE_MONTH;
    day = MODULE_RELEASE_DAY;
}

}   // namespace pcl

PCL_MODULE_EXPORT int InstallPixInsightModule(int mode)
{
    new pcl::TRTInferenceModule;

    if (mode == pcl::InstallMode::FullInstall) {
        new pcl::TRTInferenceProcess;
        new pcl::TRTInferenceInterface;
    }

    return 0;
}