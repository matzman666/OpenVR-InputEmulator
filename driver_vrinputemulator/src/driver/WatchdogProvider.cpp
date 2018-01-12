#include "WatchdogProvider.h"

#include "../logging.h"


// driver namespace
namespace vrinputemulator {
namespace driver {


vr::EVRInitError WatchdogProvider::Init(vr::IVRDriverContext * pDriverContext) {
	LOG(TRACE) << "WatchdogProvider::Init()";
	VR_INIT_WATCHDOG_DRIVER_CONTEXT(pDriverContext);
	return vr::VRInitError_None;
}


void WatchdogProvider::Cleanup() {
	LOG(TRACE) << "WatchdogProvider::Cleanup()";
	VR_CLEANUP_WATCHDOG_DRIVER_CONTEXT();
}


}
}
