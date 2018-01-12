#include "driver/ServerDriver.h"
#include "driver/WatchdogProvider.h"


vrinputemulator::driver::ServerDriver serverDriver;
vrinputemulator::driver::WatchdogProvider watchdogProvider;


#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )


HMD_DLL_EXPORT void *HmdDriverFactory(const char *pInterfaceName, int *pReturnCode) {
	LOG(TRACE) << "HmdDriverFactory( " << pInterfaceName << " )";
	if (std::strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName) == 0){
		return &serverDriver;
	} else if (std::strcmp(vr::IVRWatchdogProvider_Version, pInterfaceName) == 0) {
		return &watchdogProvider;
	}
	if (pReturnCode) {
		*pReturnCode = vr::VRInitError_Init_InterfaceNotFound;
	}
	return nullptr;
}
