#include "IVRServerDriverHost006Hooks.h"

#include "../driver/ServerDriver.h"


namespace vrinputemulator {
namespace driver {


HookData<IVRServerDriverHost006Hooks::trackedDeviceAdded_t> IVRServerDriverHost006Hooks::trackedDeviceAddedHook;
HookData<IVRServerDriverHost006Hooks::trackedDevicePoseUpdated_t> IVRServerDriverHost006Hooks::trackedDevicePoseUpdatedHook;
HookData<IVRServerDriverHost006Hooks::pollNextEvent_t> IVRServerDriverHost006Hooks::pollNextEventHook;


IVRServerDriverHost006Hooks::IVRServerDriverHost006Hooks(void* iptr) {
	if (!_isHooked) {
		CREATE_MH_HOOK(trackedDeviceAddedHook, _trackedDeviceAdded, "IVRServerDriverHost006::TrackedDeviceAdded", iptr, 0);
		CREATE_MH_HOOK(trackedDevicePoseUpdatedHook, _trackedDevicePoseUpdated, "IVRServerDriverHost006::TrackedDevicePoseUpdated", iptr, 1);
		CREATE_MH_HOOK(pollNextEventHook, _pollNextEvent, "IVRServerDriverHost006::PollNextEvent", iptr, 5);
		_isHooked = true;
	}
}


IVRServerDriverHost006Hooks::~IVRServerDriverHost006Hooks() {
	if (_isHooked) {
		REMOVE_MH_HOOK(trackedDeviceAddedHook);
		REMOVE_MH_HOOK(trackedDevicePoseUpdatedHook);
		REMOVE_MH_HOOK(pollNextEventHook);
		_isHooked = false;
	}
}


std::shared_ptr<InterfaceHooks> IVRServerDriverHost006Hooks::createHooks(void * iptr) {
	std::shared_ptr<InterfaceHooks> retval = std::shared_ptr<InterfaceHooks>(new IVRServerDriverHost006Hooks(iptr));
	return retval;
}

void IVRServerDriverHost006Hooks::trackedDevicePoseUpdatedOrig(void * _this, uint32_t unWhichDevice, const vr::DriverPose_t & newPose, uint32_t unPoseStructSize) {
	trackedDevicePoseUpdatedHook.origFunc(_this, unWhichDevice, newPose, unPoseStructSize);
}


bool IVRServerDriverHost006Hooks::_trackedDeviceAdded(void* _this, const char *pchDeviceSerialNumber, vr::ETrackedDeviceClass eDeviceClass, void *pDriver) {
	LOG(TRACE) << "IVRServerDriverHost006Hooks::_trackedDeviceAdded(" << _this << ", " << pchDeviceSerialNumber << ", " << eDeviceClass << ", " << pDriver << ")";
	serverDriver->hooksTrackedDeviceAdded(_this, 5, pchDeviceSerialNumber, eDeviceClass, pDriver);
	auto retval = trackedDeviceAddedHook.origFunc(_this, pchDeviceSerialNumber, eDeviceClass, pDriver);
	return retval;
}

void IVRServerDriverHost006Hooks::_trackedDevicePoseUpdated(void* _this, uint32_t unWhichDevice, const vr::DriverPose_t& newPose, uint32_t unPoseStructSize) {
	// Call rates:
	//
	// Vive HMD: 1120 calls/s
	// Vive Controller: 369 calls/s each
	//
	// Time is key. If we assume 1 HMD and 13 controllers, we have a total of  ~6000 calls/s. That's about 166 microseconds per call at 100% load.
	auto poseCopy = newPose;
	if (serverDriver->hooksTrackedDevicePoseUpdated(_this, 5, unWhichDevice, poseCopy, unPoseStructSize)) {
		trackedDevicePoseUpdatedHook.origFunc(_this, unWhichDevice, poseCopy, unPoseStructSize);
	}
}

bool IVRServerDriverHost006Hooks::_pollNextEvent(void* _this, void* pEvent, uint32_t uncbVREvent) {
	auto injectedEvent = serverDriver->getDriverEventForInjection(_this);
	if (injectedEvent.first) {
		if (injectedEvent.second == uncbVREvent) {
			memcpy(pEvent, injectedEvent.first.get(), uncbVREvent);
			auto event = (vr::VREvent_t*)pEvent;
			LOG(DEBUG) << "IVRServerDriverHost006Hooks::_pollNextEvent: Injecting event: " << event->eventType << ", " << event->trackedDeviceIndex;
			return true;
		} else {
			auto event = (vr::VREvent_t*)injectedEvent.first.get();
			LOG(ERROR) << "IVRServerDriverHost006Hooks::_pollNextEvent: Could not inject event (" << event->eventType << ", " << event->trackedDeviceIndex 
				<< ") because size does not match, expected " << uncbVREvent << " but got " << injectedEvent.second;
		}
	}
	bool retval, hretval;
	do {
		retval = pollNextEventHook.origFunc(_this, pEvent, uncbVREvent);
		if (retval) {
			hretval = serverDriver->hooksPollNextEvent(_this, 5, pEvent, uncbVREvent);
		}
	} while (retval && !hretval);
	return retval;
}


}
}
