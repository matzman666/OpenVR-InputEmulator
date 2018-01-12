#pragma once

#include "common.h"
#include <memory>
#include <openvr_driver.h>


namespace vrinputemulator {
namespace driver {


class IVRServerDriverHost005Hooks : public InterfaceHooks {
public:
	typedef bool(*trackedDeviceAdded_t)(void*, const char*, vr::ETrackedDeviceClass, void*);
	typedef void(*trackedDevicePoseUpdated_t)(void*, uint32_t, const vr::DriverPose_t&, uint32_t);
	typedef bool(*pollNextEvent_t)(void*, void*, uint32_t);

	static std::shared_ptr<InterfaceHooks> createHooks(void* iptr);
	virtual ~IVRServerDriverHost005Hooks();

	static void trackedDevicePoseUpdatedOrig(void* _this, uint32_t unWhichDevice, const vr::DriverPose_t& newPose, uint32_t unPoseStructSize);

private:
	bool _isHooked = false;

	IVRServerDriverHost005Hooks(void* iptr);

	static HookData<trackedDeviceAdded_t> trackedDeviceAddedHook;
	static HookData<trackedDevicePoseUpdated_t> trackedDevicePoseUpdatedHook;
	static HookData<pollNextEvent_t> pollNextEventHook;

	static bool _trackedDeviceAdded(void* _this, const char *pchDeviceSerialNumber, vr::ETrackedDeviceClass eDeviceClass, void *pDriver);
	static void _trackedDevicePoseUpdated(void* _this, uint32_t unWhichDevice, const vr::DriverPose_t& newPose, uint32_t unPoseStructSize);
	static bool _pollNextEvent(void* _this, void* pEvent, uint32_t uncbVREvent);

};

}
}
