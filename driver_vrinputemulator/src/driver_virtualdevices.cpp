
#include "stdafx.h"
#include "driver_vrinputemulator.h"

namespace vrinputemulator {
namespace driver {




CTrackedDeviceDriver::CTrackedDeviceDriver(CServerDriver* parent, VirtualDeviceType type, const std::string& serial, uint32_t virtualId)
		: m_serverDriver(parent), m_deviceType(type), m_serialNumber(serial), m_virtualDeviceId(virtualId) {
	memset(&m_pose, 0, sizeof(vr::DriverPose_t));
	m_pose.qDriverFromHeadRotation.w = 1;
	m_pose.qWorldFromDriverRotation.w = 1;
	m_pose.qRotation.w = 1;
	m_pose.result = vr::ETrackingResult::TrackingResult_Uninitialized;
}

vr::EVRInitError CTrackedDeviceDriver::Activate(uint32_t unObjectId) {
	LOG(TRACE) << "CTrackedDeviceDriver[" << m_serialNumber << "]::Activate( " << unObjectId << " )";
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	m_propertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);
	m_openvrId = unObjectId;
	m_serverDriver->_trackedDeviceActivated(m_openvrId, this);
	for (auto& e : _deviceProperties) {
		auto errorMessage = boost::apply_visitor(DevicePropertyValueVisitor(m_propertyContainer, (vr::ETrackedDeviceProperty)e.first), e.second);
		if (!errorMessage.empty()) {
			LOG(ERROR) << "Could not set tracked device property: " << errorMessage;
		}
	}
	return vr::VRInitError_None;
}


void CTrackedDeviceDriver::Deactivate() {
	LOG(TRACE) << "CTrackedDeviceDriver[" << m_serialNumber << "]::Deactivate()";
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	m_serverDriver->_trackedDeviceDeactivated(m_openvrId);
	m_openvrId = vr::k_unTrackedDeviceIndexInvalid;
}


void * CTrackedDeviceDriver::GetComponent(const char * pchComponentNameAndVersion) {
	LOG(TRACE) << "CTrackedDeviceDriver[" << m_serialNumber << "]::GetComponent( " << pchComponentNameAndVersion << " )";
	if (std::strcmp(pchComponentNameAndVersion, vr::ITrackedDeviceServerDriver_Version) == 0) {
		return static_cast<vr::ITrackedDeviceServerDriver*>(this);
	}
	return nullptr;
}


vr::DriverPose_t CTrackedDeviceDriver::GetPose() {
	LOG(TRACE) << "CTrackedDeviceDriver[" << m_serialNumber << "]::GetPose()";
	return m_pose;
}


void CTrackedDeviceDriver::updatePose(const vr::DriverPose_t & newPose, double timeOffset, bool notify) {
	LOG(TRACE) << "CTrackedDeviceDriver[" << m_serialNumber << "]::updatePose( " << timeOffset << " )";
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	m_pose = newPose;
	m_pose.poseTimeOffset += timeOffset;
	if (notify && m_openvrId != vr::k_unTrackedDeviceIndexInvalid) {
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_openvrId, m_pose, sizeof(vr::DriverPose_t));
	}
}

void CTrackedDeviceDriver::sendPoseUpdate(double timeOffset, bool onlyWhenConnected) {
	LOG(TRACE) << "CTrackedDeviceDriver[" << m_serialNumber << "]::sendPoseUpdate( " << timeOffset << " )";
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (!onlyWhenConnected || (m_pose.poseIsValid && m_pose.deviceIsConnected)) {
		m_pose.poseTimeOffset = timeOffset;
		if (m_openvrId != vr::k_unTrackedDeviceIndexInvalid) {
			vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_openvrId, m_pose, sizeof(vr::DriverPose_t));
		}
	}
}

void CTrackedDeviceDriver::publish() {
	LOG(TRACE) << "CTrackedDeviceDriver[" << m_serialNumber << "]::publish()";
	if (!m_published) {
		vr::ETrackedPropertyError pError;
		vr::ETrackedDeviceClass deviceClass = (vr::ETrackedDeviceClass)getTrackedDeviceProperty<int32_t>(vr::Prop_DeviceClass_Int32, &pError);
		if (pError == vr::TrackedProp_Success) {
			vr::VRServerDriverHost()->TrackedDeviceAdded(m_serialNumber.c_str(), deviceClass, this);
			m_published = true;
		} else {
			throw std::runtime_error(std::string("Could not get device class (") + std::to_string((int)pError) + std::string(")"));
		}
	}
}


CTrackedControllerDriver::CTrackedControllerDriver(CServerDriver* parent, const std::string& serial)
		: CTrackedDeviceDriver(parent, VirtualDeviceType::TrackedController, serial) {
	memset(&m_ControllerState, 0, sizeof(vr::VRControllerState_t));
}


vr::VRControllerState_t CTrackedControllerDriver::GetControllerState() {
	LOG(TRACE) << "CTrackedControllerDriver[" << m_serialNumber << "]::GetControllerState()";
	return m_ControllerState;
}

void * CTrackedControllerDriver::GetComponent(const char * pchComponentNameAndVersion) {
	LOG(TRACE) << "CTrackedControllerDriver[" << m_serialNumber << "]::GetComponent( " << pchComponentNameAndVersion << " )";
	if (std::strcmp(pchComponentNameAndVersion, vr::IVRControllerComponent_Version) == 0) {
		return static_cast<vr::IVRControllerComponent*>(this);
	}
	return CTrackedDeviceDriver::GetComponent(pchComponentNameAndVersion);
}

bool CTrackedControllerDriver::TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds) {
	LOG(TRACE) << "CTrackedControllerDriver[" << m_serialNumber << "]::TriggerHapticPulse( " << unAxisId << " )";
	return true; // returning false will cause errors to come out of vrserver
}

void CTrackedControllerDriver::updateControllerState(const vr::VRControllerState_t & newState, double offset, bool notify) {
	LOG(TRACE) << "CTrackedControllerDriver[" << m_serialNumber << "]::updateControllerState()";
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (notify && m_openvrId != vr::k_unTrackedDeviceIndexInvalid) {
		auto oldState = m_ControllerState;
		m_ControllerState = newState;
		// Iterate over buttons and check for state changes
		for (unsigned i = 0; i < vr::k_EButton_Max; ++i) {
			auto oldTouchedState = oldState.ulButtonTouched & vr::ButtonMaskFromId((vr::EVRButtonId)i);
			auto newTouchedState = newState.ulButtonTouched & vr::ButtonMaskFromId((vr::EVRButtonId)i);
			if (!oldTouchedState && newTouchedState) {
				LOG(DEBUG) << m_serialNumber << ": ButtonTouch detected: " << i;
				vr::VRServerDriverHost()->TrackedDeviceButtonTouched(m_openvrId, (vr::EVRButtonId)i, offset);
			} else if (oldTouchedState && !newTouchedState) {
				LOG(DEBUG) << m_serialNumber << ": ButtonUntouch detected: " << i;
				vr::VRServerDriverHost()->TrackedDeviceButtonUntouched(m_openvrId, (vr::EVRButtonId)i, offset);
			}
			auto oldPressedState = oldState.ulButtonPressed & vr::ButtonMaskFromId((vr::EVRButtonId)i);
			auto newPressedState = newState.ulButtonPressed & vr::ButtonMaskFromId((vr::EVRButtonId)i);
			if (!oldPressedState && newPressedState) {
				LOG(DEBUG) << m_serialNumber << ": ButtonPress detected: " << i;
				vr::VRServerDriverHost()->TrackedDeviceButtonPressed(m_openvrId, (vr::EVRButtonId)i, offset);
			} else if (oldPressedState && !newPressedState) {
				LOG(DEBUG) << m_serialNumber << ": ButtonUnpress detected: " << i;
				vr::VRServerDriverHost()->TrackedDeviceButtonUnpressed(m_openvrId, (vr::EVRButtonId)i, offset);
			}
		}
		// Iterate over axis and check for state changes
		for (unsigned i = 0; i < vr::k_unControllerStateAxisCount; ++i) {
			if (oldState.rAxis[i].x != newState.rAxis[i].x || oldState.rAxis[i].y != newState.rAxis[i].y) {
				LOG(DEBUG) << m_serialNumber << ": AxisChange detected: " << i;
				vr::VRServerDriverHost()->TrackedDeviceAxisUpdated(m_openvrId, i, newState.rAxis[i]);
			}
		}
	} else {
		m_ControllerState = newState;
	}
}

void CTrackedControllerDriver::buttonEvent(ButtonEventType eventType, uint32_t buttonId, double timeOffset, bool notify) {
	LOG(TRACE) << "CTrackedControllerDriver[" << m_serialNumber << "]::buttonEvent( " << (int)eventType << ", " << buttonId << ", " << timeOffset << " )";
	switch (eventType) {
		case ButtonEventType::ButtonPressed:
			m_ControllerState.ulButtonPressed |= vr::ButtonMaskFromId((vr::EVRButtonId)buttonId);
			if (notify && m_openvrId != vr::k_unTrackedDeviceIndexInvalid) {
				vr::VRServerDriverHost()->TrackedDeviceButtonPressed(m_openvrId, (vr::EVRButtonId)buttonId, timeOffset);
			}
			break;
		case ButtonEventType::ButtonUnpressed:
			m_ControllerState.ulButtonPressed &= ~vr::ButtonMaskFromId((vr::EVRButtonId)buttonId);
			if (notify && m_openvrId != vr::k_unTrackedDeviceIndexInvalid) {
				vr::VRServerDriverHost()->TrackedDeviceButtonUnpressed(m_openvrId, (vr::EVRButtonId)buttonId, timeOffset);
			}
			break;
		case ButtonEventType::ButtonTouched:
			m_ControllerState.ulButtonTouched |= vr::ButtonMaskFromId((vr::EVRButtonId)buttonId);
			if (notify && m_openvrId != vr::k_unTrackedDeviceIndexInvalid) {
				vr::VRServerDriverHost()->TrackedDeviceButtonTouched(m_openvrId, (vr::EVRButtonId)buttonId, timeOffset);
			}
			break;
		case ButtonEventType::ButtonUntouched:
			m_ControllerState.ulButtonTouched &= ~vr::ButtonMaskFromId((vr::EVRButtonId)buttonId);
			if (notify && m_openvrId != vr::k_unTrackedDeviceIndexInvalid) {
				vr::VRServerDriverHost()->TrackedDeviceButtonUntouched(m_openvrId, (vr::EVRButtonId)buttonId, timeOffset);
			}
			break;
		default:
			break;
	}
}

void CTrackedControllerDriver::axisEvent(uint32_t axisId, const vr::VRControllerAxis_t & axisState, bool notify) {
	LOG(TRACE) << "CTrackedControllerDriver[" << m_serialNumber << "]::axisEvent( " << axisId << " )";
	if (axisId < vr::k_unControllerStateAxisCount) {
		m_ControllerState.rAxis[axisId] = axisState;
		if (notify && m_openvrId != vr::k_unTrackedDeviceIndexInvalid) {
			vr::VRServerDriverHost()->TrackedDeviceAxisUpdated(m_openvrId, axisId, m_ControllerState.rAxis[axisId]);
		}
	}
}

} // end namespace driver
} // end namespace vrinputemulator
