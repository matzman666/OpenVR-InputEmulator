#pragma once

#include "common.h"
#include <memory>
#include <openvr_driver.h>


namespace vrinputemulator {
namespace driver {


class IVRServerDriverHost004Hooks : public InterfaceHooks {
public:
	typedef bool(*trackedDeviceAdded_t)(void*, const char*, vr::ETrackedDeviceClass, void*);
	typedef void(*trackedDevicePoseUpdated_t)(void*, uint32_t, const vr::DriverPose_t&, uint32_t);
	typedef void(*trackedDeviceButtonPressed_t)(void*, uint32_t, vr::EVRButtonId, double);
	typedef void(*trackedDeviceButtonUnpressed_t)(void*, uint32_t, vr::EVRButtonId, double);
	typedef void(*trackedDeviceButtonTouched_t)(void*, uint32_t, vr::EVRButtonId, double);
	typedef void(*trackedDeviceButtonUntouched_t)(void*, uint32_t, vr::EVRButtonId, double);
	typedef void(*trackedDeviceAxisUpdated_t)(void*, uint32_t, uint32_t, const vr::VRControllerAxis_t&);

	static std::shared_ptr<InterfaceHooks> createHooks(void* iptr);
	virtual ~IVRServerDriverHost004Hooks();

	static void trackedDevicePoseUpdatedOrig(void* _this, uint32_t unWhichDevice, const vr::DriverPose_t& newPose, uint32_t unPoseStructSize);
	static void trackedDeviceButtonPressedOrig(void* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);
	static void trackedDeviceButtonUnpressedOrig(void* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);
	static void trackedDeviceButtonTouchedOrig(void* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);
	static void trackedDeviceButtonUntouchedOrig(void* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);
	static void trackedDeviceAxisUpdatedOrig(void* _this, uint32_t unWhichDevice, uint32_t unWhichAxis, const vr::VRControllerAxis_t & axisState);

private:
	bool _isHooked = false;

	IVRServerDriverHost004Hooks(void* iptr);

	static HookData<trackedDeviceAdded_t> trackedDeviceAddedHook;
	static HookData<trackedDevicePoseUpdated_t> trackedDevicePoseUpdatedHook;
	static HookData<trackedDeviceButtonPressed_t> trackedDeviceButtonPressedHook;
	static HookData<trackedDeviceButtonUnpressed_t> trackedDeviceButtonUnpressedHook;
	static HookData<trackedDeviceButtonTouched_t> trackedDeviceButtonTouchedHook;
	static HookData<trackedDeviceButtonUntouched_t> trackedDeviceButtonUntouchedHook;
	static HookData<trackedDeviceAxisUpdated_t> trackedDeviceAxisUpdatedHook;

	static bool _trackedDeviceAdded(void* _this, const char *pchDeviceSerialNumber, vr::ETrackedDeviceClass eDeviceClass, void *pDriver);
	static void _trackedDevicePoseUpdated(void* _this, uint32_t unWhichDevice, const vr::DriverPose_t& newPose, uint32_t unPoseStructSize);
	static void _trackedDeviceButtonPressed(void* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);
	static void _trackedDeviceButtonUnpressed(void* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);
	static void _trackedDeviceButtonTouched(void* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);
	static void _trackedDeviceButtonUntouched(void* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);
	static void _trackedDeviceAxisUpdated(void* _this, uint32_t unWhichDevice, uint32_t unWhichAxis, const vr::VRControllerAxis_t & axisState);

};

}
}

