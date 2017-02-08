
#include "stdafx.h"
#include "driver_vrinputemulator.h"


#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )

namespace vrinputemulator {
namespace driver {

CServerDriver serverDriver;
CWatchdogProvider watchdogProvider;


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


vr::EVRInitError CWatchdogProvider::Init(vr::IVRDriverContext * pDriverContext) {
	LOG(TRACE) << "CWatchdogProvider::Init()";
	VR_INIT_WATCHDOG_DRIVER_CONTEXT(pDriverContext);
	return vr::VRInitError_None;
}


void CWatchdogProvider::Cleanup() {
	LOG(TRACE) << "CWatchdogProvider::Cleanup()";
	VR_CLEANUP_WATCHDOG_DRIVER_CONTEXT();
}

} // end namespace driver
} // end namespace vrinputemulator
