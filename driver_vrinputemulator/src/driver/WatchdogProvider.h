#pragma once

#include <openvr_driver.h>


// driver namespace
namespace vrinputemulator {
namespace driver {


/**
* Implements the IVRWatchdogProvider interface.
*
* Its only purpose seems to be to start SteamVR by calling WatchdogWakeUp() from the IVRWatchdogHost interface.
* Valve probably uses this interface to start SteamVR whenever a button is pressed on the controller or hmd.
* We must implement it but currently we don't use it for anything.
*/
class WatchdogProvider : public vr::IVRWatchdogProvider {
public:
	/** initializes the driver in watchdog mode. */
	virtual vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext) override;

	/** cleans up the driver right before it is unloaded */
	virtual void Cleanup() override;
};


}
}
