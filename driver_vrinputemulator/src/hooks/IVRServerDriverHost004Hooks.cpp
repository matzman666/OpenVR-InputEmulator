#include "IVRServerDriverHost004Hooks.h"

#include "../driver/ServerDriver.h"


namespace vrinputemulator {
namespace driver {


HookData<IVRServerDriverHost004Hooks::trackedDeviceAdded_t> IVRServerDriverHost004Hooks::trackedDeviceAddedHook;
HookData<IVRServerDriverHost004Hooks::trackedDevicePoseUpdated_t> IVRServerDriverHost004Hooks::trackedDevicePoseUpdatedHook;
HookData<IVRServerDriverHost004Hooks::trackedDeviceButtonPressed_t> IVRServerDriverHost004Hooks::trackedDeviceButtonPressedHook;
HookData<IVRServerDriverHost004Hooks::trackedDeviceButtonUnpressed_t> IVRServerDriverHost004Hooks::trackedDeviceButtonUnpressedHook;
HookData<IVRServerDriverHost004Hooks::trackedDeviceButtonTouched_t> IVRServerDriverHost004Hooks::trackedDeviceButtonTouchedHook;
HookData<IVRServerDriverHost004Hooks::trackedDeviceButtonUntouched_t> IVRServerDriverHost004Hooks::trackedDeviceButtonUntouchedHook;
HookData<IVRServerDriverHost004Hooks::trackedDeviceAxisUpdated_t> IVRServerDriverHost004Hooks::trackedDeviceAxisUpdatedHook;


IVRServerDriverHost004Hooks::IVRServerDriverHost004Hooks(void* iptr) {
	if (!_isHooked) {
		CREATE_MH_HOOK(trackedDeviceAddedHook, _trackedDeviceAdded, "IVRServerDriverHost004::TrackedDeviceAdded", iptr, 0);
		CREATE_MH_HOOK(trackedDevicePoseUpdatedHook, _trackedDevicePoseUpdated, "IVRServerDriverHost004::TrackedDevicePoseUpdated", iptr, 1);
		CREATE_MH_HOOK(trackedDeviceButtonPressedHook, _trackedDeviceButtonPressed, "IVRServerDriverHost004::TrackedDeviceButtonPressed", iptr, 3);
		CREATE_MH_HOOK(trackedDeviceButtonUnpressedHook, _trackedDeviceButtonUnpressed, "IVRServerDriverHost004::TrackedDeviceButtonUnpressed", iptr, 4);
		CREATE_MH_HOOK(trackedDeviceButtonTouchedHook, _trackedDeviceButtonTouched, "IVRServerDriverHost004::TrackedDeviceButtonTouched", iptr, 5);
		CREATE_MH_HOOK(trackedDeviceButtonUntouchedHook, _trackedDeviceButtonUntouched, "IVRServerDriverHost004::TrackedDeviceButtonUntouched", iptr, 6);
		CREATE_MH_HOOK(trackedDeviceAxisUpdatedHook, _trackedDeviceAxisUpdated, "IVRServerDriverHost004::TrackedDeviceAxisUpdated", iptr, 7);
		_isHooked = true;
	}
}


IVRServerDriverHost004Hooks::~IVRServerDriverHost004Hooks() {
	if (_isHooked) {
		REMOVE_MH_HOOK(trackedDeviceAddedHook);
		REMOVE_MH_HOOK(trackedDevicePoseUpdatedHook);
		REMOVE_MH_HOOK(trackedDeviceButtonPressedHook);
		REMOVE_MH_HOOK(trackedDeviceButtonUnpressedHook);
		REMOVE_MH_HOOK(trackedDeviceButtonTouchedHook);
		REMOVE_MH_HOOK(trackedDeviceButtonUntouchedHook);
		REMOVE_MH_HOOK(trackedDeviceAxisUpdatedHook);
		_isHooked = false;
	}
}


std::shared_ptr<InterfaceHooks> IVRServerDriverHost004Hooks::createHooks(void * iptr) {
	std::shared_ptr<InterfaceHooks> retval = std::shared_ptr<InterfaceHooks>(new IVRServerDriverHost004Hooks(iptr));
	return retval;
}

void IVRServerDriverHost004Hooks::trackedDevicePoseUpdatedOrig(void * _this, uint32_t unWhichDevice, const vr::DriverPose_t& newPose, uint32_t unPoseStructSize) {
	trackedDevicePoseUpdatedHook.origFunc(_this, unWhichDevice, newPose, unPoseStructSize);
}

void IVRServerDriverHost004Hooks::trackedDeviceButtonPressedOrig(void * _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	trackedDeviceButtonPressedHook.origFunc(_this, unWhichDevice, eButtonId, eventTimeOffset);
}

void IVRServerDriverHost004Hooks::trackedDeviceButtonUnpressedOrig(void * _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	trackedDeviceButtonUnpressedHook.origFunc(_this, unWhichDevice, eButtonId, eventTimeOffset);
}

void IVRServerDriverHost004Hooks::trackedDeviceButtonTouchedOrig(void * _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	trackedDeviceButtonTouchedHook.origFunc(_this, unWhichDevice, eButtonId, eventTimeOffset);
}

void IVRServerDriverHost004Hooks::trackedDeviceButtonUntouchedOrig(void * _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	trackedDeviceButtonUntouchedHook.origFunc(_this, unWhichDevice, eButtonId, eventTimeOffset);
}

void IVRServerDriverHost004Hooks::trackedDeviceAxisUpdatedOrig(void * _this, uint32_t unWhichDevice, uint32_t unWhichAxis, const vr::VRControllerAxis_t & axisState) {
	trackedDeviceAxisUpdatedHook.origFunc(_this, unWhichDevice, unWhichAxis, axisState);
}


bool IVRServerDriverHost004Hooks::_trackedDeviceAdded(void* _this, const char *pchDeviceSerialNumber, vr::ETrackedDeviceClass eDeviceClass, void *pDriver) {
	char *sn = (char*)pchDeviceSerialNumber;
	if ((sn >= (char*)0 && sn < (char*)0xff) || eDeviceClass < 0 || eDeviceClass > vr::ETrackedDeviceClass::TrackedDeviceClass_DisplayRedirect ) {
		// SteamVR Vive driver bug, it's calling this function with random garbage
		LOG(ERROR) << "Not running _trackedDeviceAdded because of SteamVR driver bug.";
		return false;
	}
	LOG(TRACE) << "IVRServerDriverHost004Hooks::_trackedDeviceAdded(" << _this << ", " << pchDeviceSerialNumber << ", " << eDeviceClass << ", " << pDriver << ")";
	serverDriver->hooksTrackedDeviceAdded(_this, 4, pchDeviceSerialNumber, eDeviceClass, pDriver);
	auto retval = trackedDeviceAddedHook.origFunc(_this, pchDeviceSerialNumber, eDeviceClass, pDriver);
	return retval;
}

void IVRServerDriverHost004Hooks::_trackedDevicePoseUpdated(void* _this, uint32_t unWhichDevice, const vr::DriverPose_t& newPose, uint32_t unPoseStructSize) {
	// Call rates:
	//
	// Vive HMD: 1120 calls/s
	// Vive Controller: 369 calls/s each
	//
	// Time is key. If we assume 1 HMD and 13 controllers, we have a total of  ~6000 calls/s. That's about 166 microseconds per call at 100% load.
	auto poseCopy = newPose;
	if (serverDriver->hooksTrackedDevicePoseUpdated(_this, 4, unWhichDevice, poseCopy, unPoseStructSize)) {
		trackedDevicePoseUpdatedHook.origFunc(_this, unWhichDevice, poseCopy, unPoseStructSize);
	}
}

void IVRServerDriverHost004Hooks::_trackedDeviceButtonPressed(void* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	if (serverDriver->hooksTrackedDeviceButtonPressed(_this, 4, unWhichDevice, eButtonId, eventTimeOffset)) {
		trackedDeviceButtonPressedHook.origFunc(_this, unWhichDevice, eButtonId, eventTimeOffset);
	}
}

void IVRServerDriverHost004Hooks::_trackedDeviceButtonUnpressed(void* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	if (serverDriver->hooksTrackedDeviceButtonUnpressed(_this, 4, unWhichDevice, eButtonId, eventTimeOffset)) {
		trackedDeviceButtonUnpressedHook.origFunc(_this, unWhichDevice, eButtonId, eventTimeOffset);
	}
}

void IVRServerDriverHost004Hooks::_trackedDeviceButtonTouched(void* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	if (serverDriver->hooksTrackedDeviceButtonTouched(_this, 4, unWhichDevice, eButtonId, eventTimeOffset)) {
		trackedDeviceButtonTouchedHook.origFunc(_this, unWhichDevice, eButtonId, eventTimeOffset);
	}
}

void IVRServerDriverHost004Hooks::_trackedDeviceButtonUntouched(void* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	if (serverDriver->hooksTrackedDeviceButtonUntouched(_this, 4, unWhichDevice, eButtonId, eventTimeOffset)) {
		trackedDeviceButtonUntouchedHook.origFunc(_this, unWhichDevice, eButtonId, eventTimeOffset);
	}
}

void IVRServerDriverHost004Hooks::_trackedDeviceAxisUpdated(void* _this, uint32_t unWhichDevice, uint32_t unWhichAxis, const vr::VRControllerAxis_t & axisState) {
	auto stateCopy = axisState;
	if (serverDriver->hooksTrackedDeviceAxisUpdated(_this, 4, unWhichDevice, unWhichAxis, stateCopy)) {
		trackedDeviceAxisUpdatedHook.origFunc(_this, unWhichDevice, unWhichAxis, stateCopy);
	}
}


}
}
