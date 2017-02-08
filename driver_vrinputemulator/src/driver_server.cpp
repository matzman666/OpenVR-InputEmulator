
#include "stdafx.h"
#include "driver_vrinputemulator.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <ipc_protocol.h>
#include <openvr_math.h>
#include <MinHook.h>
#include <map>
#include <vector>

namespace vrinputemulator {
namespace driver {


#define CREATE_MH_HOOK(detourInfo, detourFunc, logName, objPtr, vtableOffset) \
	detourInfo.targetFunc = (*((void***)objPtr))[vtableOffset]; \
	mhError = MH_CreateHook(detourInfo.targetFunc, (void*)&detourFunc, reinterpret_cast<LPVOID*>(&detourInfo.origFunc)); \
	if (mhError == MH_OK) { \
		mhError = MH_EnableHook(detourInfo.targetFunc); \
		if (mhError == MH_OK) { \
			detourInfo.enabled = true; \
			LOG(INFO) << logName << " hook is enabled"; \
		} else { \
			MH_RemoveHook(detourInfo.targetFunc); \
			LOG(ERROR) << "Error while enabling " << logName << " hook: " << MH_StatusToString(mhError); \
		} \
	} else { \
		LOG(ERROR) << "Error while creating " << logName << " hook: " << MH_StatusToString(mhError); \
	}


#define REMOVE_MH_HOOK(detourInfo) \
	if (detourInfo.enabled) { \
		MH_RemoveHook(detourInfo.targetFunc); \
		detourInfo.enabled = false; \
	}


CServerDriver* CServerDriver::singleton = nullptr;

std::map<vr::ITrackedDeviceServerDriver*, CServerDriver::_TrackedDeviceInfo> CServerDriver::_trackedDeviceInfoMap;
CServerDriver::_TrackedDeviceInfo* CServerDriver::_trackedDeviceInfos[vr::k_unMaxTrackedDeviceCount]; // index == openvrId

CServerDriver::_DetourFuncInfo<CServerDriver::_DetourTrackedDeviceAdded_t> CServerDriver::_deviceAddedDetour;
CServerDriver::_DetourFuncInfo<CServerDriver::_DetourTrackedDevicePoseUpdated_t> CServerDriver::_poseUpatedDetour;
CServerDriver::_DetourFuncInfo<CServerDriver::_DetourTrackedDeviceButtonPressed_t> CServerDriver::_buttonPressedDetour;
CServerDriver::_DetourFuncInfo<CServerDriver::_DetourTrackedDeviceButtonUnpressed_t> CServerDriver::_buttonUnpressedDetour;
CServerDriver::_DetourFuncInfo<CServerDriver::_DetourTrackedDeviceButtonTouched_t> CServerDriver::_buttonTouchedDetour;
CServerDriver::_DetourFuncInfo<CServerDriver::_DetourTrackedDeviceButtonUntouched_t> CServerDriver::_buttonUntouchedDetour;
CServerDriver::_DetourFuncInfo<CServerDriver::_DetourTrackedDeviceAxisUpdated_t> CServerDriver::_axisUpdatedDetour;
std::vector<CServerDriver::_DetourFuncInfo<CServerDriver::_DetourTrackedDeviceActivate_t>> CServerDriver::_deviceActivateDetours;
std::map<vr::ITrackedDeviceServerDriver*, CServerDriver::_DetourFuncInfo<CServerDriver::_DetourTrackedDeviceActivate_t>*> CServerDriver::_deviceActivateDetourMap; // _this => DetourInfo


CServerDriver::~CServerDriver() {
	LOG(TRACE) << "CServerDriver::~CServerDriver_InputEmulator()";
	// 10/10/2015 benj:  vrserver is exiting without calling Cleanup() to balance Init()
	// causing std::thread to call std::terminate

	// Apparently the above comment is not true anymore
	// Cleanup(); 
}


void CServerDriver::_poseUpatedDetourFunc(vr::IVRServerDriverHost* _this, uint32_t unWhichDevice, const vr::DriverPose_t& newPose, uint32_t unPoseStructSize) {
	// Call rates:
	//
	// Vive HMD: 1120 calls/s
	// Vive Controller: 369 calls/s each
	//
	// Time is key. If we assume 1 HMD and 13 controllers, we have a total of  ~6000 calls/s. That's about 166 microseconds per call at 100% load.

	/*if (unWhichDevice == 0) {
		#define CALLCOUNTSAMPESIZE 1000
		static uint64_t callcount = 0;
		static uint64_t starttime = 0;
		callcount++;
		if (starttime == 0) {
			starttime = std::chrono::duration_cast <std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		} else if (callcount > CALLCOUNTSAMPESIZE) {
			auto endtime = std::chrono::duration_cast <std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			auto diff = endtime - starttime;
			LOG(INFO) << "HMD pose update: " << CALLCOUNTSAMPESIZE << " calls in " << diff << "ms: " << CALLCOUNTSAMPESIZE * 1000 / diff << " calls / s";
			starttime = std::chrono::duration_cast <std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			callcount = 0;
		}
	}*/

	if (_trackedDeviceInfos[unWhichDevice] && _trackedDeviceInfos[unWhichDevice]->isValid) {
		vr::DriverPose_t nPose = newPose;
		if (_trackedDeviceInfos[unWhichDevice]->enablePoseOffset) {
			nPose.vecWorldFromDriverTranslation[0] += _trackedDeviceInfos[unWhichDevice]->poseOffset[0];
			nPose.vecWorldFromDriverTranslation[1] += _trackedDeviceInfos[unWhichDevice]->poseOffset[1];
			nPose.vecWorldFromDriverTranslation[2] += _trackedDeviceInfos[unWhichDevice]->poseOffset[2];
		}
		if (_trackedDeviceInfos[unWhichDevice]->enablePoseRotation) {
			nPose.qDriverFromHeadRotation = _trackedDeviceInfos[unWhichDevice]->poseRotation * nPose.qDriverFromHeadRotation;
		}
		if (_trackedDeviceInfos[unWhichDevice]->mirrorMode == 1) {
			auto targetId = _trackedDeviceInfos[unWhichDevice]->mirrorTarget;
			if (targetId < vr::k_unMaxTrackedDeviceCount && _trackedDeviceInfos[targetId] && _trackedDeviceInfos[targetId]->isValid) {
				_poseUpatedDetourFunc(_trackedDeviceInfos[targetId]->driverHost, _trackedDeviceInfos[unWhichDevice]->mirrorTarget, newPose, unPoseStructSize);
			}
			return _poseUpatedDetour.origFunc(_this, unWhichDevice, nPose, unPoseStructSize);
		} else if (_trackedDeviceInfos[unWhichDevice]->mirrorMode == 2) {
			auto targetId = _trackedDeviceInfos[unWhichDevice]->mirrorTarget;
			if (targetId < vr::k_unMaxTrackedDeviceCount && _trackedDeviceInfos[targetId] && _trackedDeviceInfos[targetId]->isValid) {
				return _poseUpatedDetourFunc(_trackedDeviceInfos[targetId]->driverHost, _trackedDeviceInfos[unWhichDevice]->mirrorTarget, newPose, unPoseStructSize);
			}
		} else {
			return _poseUpatedDetour.origFunc(_this, unWhichDevice, nPose, unPoseStructSize);
		}
	} else {
		return _poseUpatedDetour.origFunc(_this, unWhichDevice, newPose, unPoseStructSize);
	}
}

void CServerDriver::_buttonPressedDetourFunc(vr::IVRServerDriverHost* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	LOG(INFO) << "Detour::buttonPressedDetourFunc(" << _this << ", " << unWhichDevice << ", " << (int)eButtonId << ", " << eventTimeOffset << ")";
	vr::EVRButtonId button = eButtonId;
	if (_trackedDeviceInfos[unWhichDevice] && _trackedDeviceInfos[unWhichDevice]->isValid) {
		_trackedDeviceInfos[unWhichDevice]->getButtonMapping(eButtonId, button);
	}
	return _buttonPressedDetour.origFunc(_this, unWhichDevice, button, eventTimeOffset);
}

void CServerDriver::_buttonUnpressedDetourFunc(vr::IVRServerDriverHost* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	LOG(INFO) << "Detour::buttonUnpressedDetourFunc(" << _this << ", " << unWhichDevice << ", " << (int)eButtonId << ", " << eventTimeOffset << ")";
	vr::EVRButtonId button = eButtonId;
	if (_trackedDeviceInfos[unWhichDevice] && _trackedDeviceInfos[unWhichDevice]->isValid) {
		_trackedDeviceInfos[unWhichDevice]->getButtonMapping(eButtonId, button);
	}
	return _buttonUnpressedDetour.origFunc(_this, unWhichDevice, button, eventTimeOffset);
}

void CServerDriver::CServerDriver::CServerDriver::_buttonTouchedDetourFunc(vr::IVRServerDriverHost* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	LOG(INFO) << "Detour::buttonTouchedDetourFunc(" << _this << ", " << unWhichDevice << ", " << (int)eButtonId << ", " << eventTimeOffset << ")";
	vr::EVRButtonId button = eButtonId;
	if (_trackedDeviceInfos[unWhichDevice] && _trackedDeviceInfos[unWhichDevice]->isValid) {
		_trackedDeviceInfos[unWhichDevice]->getButtonMapping(eButtonId, button);
	}
	return _buttonTouchedDetour.origFunc(_this, unWhichDevice, button, eventTimeOffset);
}

void CServerDriver::_buttonUntouchedDetourFunc(vr::IVRServerDriverHost* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	LOG(INFO) << "Detour::buttonUntouchedDetourFunc(" << _this << ", " << unWhichDevice << ", " << (int)eButtonId << ", " << eventTimeOffset << ")";
	vr::EVRButtonId button = eButtonId;
	if (_trackedDeviceInfos[unWhichDevice] && _trackedDeviceInfos[unWhichDevice]->isValid) {
		_trackedDeviceInfos[unWhichDevice]->getButtonMapping(eButtonId, button);
	}
	return _buttonUntouchedDetour.origFunc(_this, unWhichDevice, button, eventTimeOffset);
}

void CServerDriver::_axisUpdatedDetourFunc(vr::IVRServerDriverHost* _this, uint32_t unWhichDevice, uint32_t unWhichAxis, const vr::VRControllerAxis_t & axisState) {
	LOG(INFO) << "Detour::axisUpdatedDetourFunc(" << _this << ", " << unWhichDevice << ", " << (int)unWhichAxis << ", <state>)";
	return _axisUpdatedDetour.origFunc(_this, unWhichDevice, unWhichAxis, axisState);
}

vr::EVRInitError CServerDriver::_deviceActivateDetourFunc(vr::ITrackedDeviceServerDriver* _this, uint32_t unObjectId) {
	LOG(INFO) << "Detour::deviceActivateDetourFunc(" << _this << ", " << unObjectId << ")";
	auto i = _trackedDeviceInfoMap.find(_this);
	if (i != _trackedDeviceInfoMap.end()) {
		auto& info = i->second;
		info.openvrId = unObjectId;
		_trackedDeviceInfos[unObjectId] = &info;
		LOG(INFO) << "Detour::deviceActivateDetourFunc: sucessfully added to trackedDeviceInfos";
	}
	auto& d = _deviceActivateDetourMap.find(_this);
	if (d != _deviceActivateDetourMap.end()) {
		return d->second->origFunc(_this, unObjectId);
	}
	return vr::VRInitError_Unknown;
}


bool CServerDriver::_deviceAddedDetourFunc(vr::IVRServerDriverHost* _this, const char *pchDeviceSerialNumber, vr::ETrackedDeviceClass eDeviceClass, vr::ITrackedDeviceServerDriver *pDriver) {
	LOG(INFO) << "Detour::deviceAddedDetourFunc(" << _this << ", " << pchDeviceSerialNumber << ", " << (int)eDeviceClass << ", " << pDriver << ")";
	auto targetFunc = (*((void***)pDriver))[0];
	_DetourFuncInfo<_DetourTrackedDeviceActivate_t>* foundEntry = nullptr;
	for (auto& d : _deviceActivateDetours) {
		if (d.targetFunc == targetFunc) {
			foundEntry = &d;
			break;
		}
	}
	if (!foundEntry) {
		_deviceActivateDetours.emplace_back();
		auto& d = _deviceActivateDetours[_deviceActivateDetours.size() - 1];
		MH_STATUS mhError;
		CREATE_MH_HOOK(d, _deviceActivateDetourFunc, "deviceActivateDetour", pDriver, 0);
		foundEntry = &d;
	}
	_deviceActivateDetourMap.insert({pDriver, foundEntry});
	_TrackedDeviceInfo info = { pDriver, eDeviceClass, pchDeviceSerialNumber, vr::k_unTrackedDeviceIndexInvalid, _this };
	_trackedDeviceInfoMap.emplace(pDriver, std::move(info));
	return _deviceAddedDetour.origFunc(_this, pchDeviceSerialNumber, eDeviceClass, pDriver);
};


vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext *pDriverContext) {
	LOG(TRACE) << "CServerDriver::Init()";
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

	auto mhError = MH_Initialize();
	if (mhError == MH_OK) {
		CREATE_MH_HOOK(_deviceAddedDetour, _deviceAddedDetourFunc, "deviceAddedDetour", vr::VRServerDriverHost(), 0);
		CREATE_MH_HOOK(_poseUpatedDetour, _poseUpatedDetourFunc, "poseUpatedDetour", vr::VRServerDriverHost(), 1);
		CREATE_MH_HOOK(_buttonPressedDetour, _buttonPressedDetourFunc, "buttonPressedDetour", vr::VRServerDriverHost(), 3);
		CREATE_MH_HOOK(_buttonUnpressedDetour, _buttonUnpressedDetourFunc, "buttonUnpressedDetour", vr::VRServerDriverHost(), 4);
		CREATE_MH_HOOK(_buttonTouchedDetour, _buttonTouchedDetourFunc, "buttonTouchedDetour", vr::VRServerDriverHost(), 5);
		CREATE_MH_HOOK(_buttonUntouchedDetour, _buttonUntouchedDetourFunc, "buttonUntouchedDetour", vr::VRServerDriverHost(), 6);
		CREATE_MH_HOOK(_axisUpdatedDetour, _axisUpdatedDetourFunc, "axisUpdatedDetour", vr::VRServerDriverHost(), 7);
	} else {
		LOG(ERROR) << "Error while initialising minHook: " << MH_StatusToString(mhError);
	}

	// Start IPC thread
	_ipcThreadStopFlag = false;
	_ipcThread = std::thread(_ipcThreadFunc, this);
	return vr::VRInitError_None;
}

void CServerDriver::Cleanup() {
	LOG(TRACE) << "CServerDriver::Cleanup()";
	std::lock_guard<std::recursive_mutex> lock(_mutex);

	REMOVE_MH_HOOK(_deviceAddedDetour);
	REMOVE_MH_HOOK(_poseUpatedDetour);
	REMOVE_MH_HOOK(_buttonPressedDetour);
	REMOVE_MH_HOOK(_buttonUnpressedDetour);
	REMOVE_MH_HOOK(_buttonTouchedDetour);
	REMOVE_MH_HOOK(_buttonUntouchedDetour);
	REMOVE_MH_HOOK(_axisUpdatedDetour);

	MH_Uninitialize();
	if (_ipcThreadRunning) {
		_ipcThreadStopFlag = true;
		_ipcThread.join();
	}
	VR_CLEANUP_SERVER_DRIVER_CONTEXT();
}

void CServerDriver::RunFrame() {
}

int32_t CServerDriver::addTrackedDevice(VirtualDeviceType type, const std::string& serial) {
	LOG(TRACE) << "CServerDriver::addTrackedDeviceDriver( " << serial << " )";
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (m_emulatedDeviceCount >= vr::k_unMaxTrackedDeviceCount) {
		return -1;
	}
	for (uint32_t i = 0; i < m_emulatedDeviceCount; ++i) {
		if (m_emulatedDevices[i]->serialNumber().compare(serial) == 0) {
			return -2;
		}
	}
	switch (type) {
		case VirtualDeviceType::TrackedController:
			{
				auto emulatedDeviceId = m_emulatedDeviceCount++;
				m_emulatedDevices[emulatedDeviceId] = std::make_shared<CTrackedControllerDriver>(this, serial);
				LOG(INFO) << "Added new tracked controller:  type " << (int)type << ", serial \"" << serial << "\", emulatedDeviceId " << emulatedDeviceId;
				return emulatedDeviceId;
			}
		default:
			return -3;
	}
}

int32_t CServerDriver::publishTrackedDevice(uint32_t emulatedDeviceId, bool notify) {
	LOG(TRACE) << "CServerDriver::publishEmulatedTrackedDevice( " << emulatedDeviceId << " )";
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (emulatedDeviceId >= m_emulatedDeviceCount) {
		return -1;
	} else if (!m_emulatedDevices[emulatedDeviceId]) {
		return -2;
	} else {
		auto device = m_emulatedDevices[emulatedDeviceId].get();
		for (uint32_t i = 0; i < m_publishedDeviceCount; ++i) {
			if (m_publishedDevices[i] == device) {
				return -3;
			}
		}
		auto publishedId = m_publishedDeviceCount++;
		m_publishedDevices[publishedId] = device;
		if (notify && vr::VRServerDriverHost()) {
			vr::ETrackedPropertyError pError;
			vr::ETrackedDeviceClass deviceClass = (vr::ETrackedDeviceClass)device->getTrackedDeviceProperty<int32_t>(vr::Prop_DeviceClass_Int32, &pError);
			if (pError == vr::TrackedProp_Success) {
				vr::VRServerDriverHost()->TrackedDeviceAdded(m_publishedDevices[publishedId]->serialNumber().c_str(), deviceClass, device);
				LOG(INFO) << "Published tracked controller: emulatedDeviceId " << emulatedDeviceId;
			} else {
				LOG(ERROR) << "Error while publishing controller: Missing device class (" << (int)pError << ")";
				return -4;
			}
		}
		return publishedId;
	}
}

void CServerDriver::trackedDeviceActivated(uint32_t deviceId, CTrackedDeviceDriver * device) {
	m_deviceIdMap[deviceId] = device;
}

void CServerDriver::trackedDeviceDeactivated(uint32_t deviceId) {
	m_deviceIdMap[deviceId] = nullptr;
}

} // end namespace driver
} // end namespace vrinputemulator
