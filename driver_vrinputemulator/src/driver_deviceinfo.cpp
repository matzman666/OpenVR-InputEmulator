
#include "stdafx.h"
#include "driver_vrinputemulator.h"
#include <openvr_math.h>


namespace vrinputemulator {
namespace driver {


#define VECTOR_ADD(lhs, rhs) \
		lhs[0] += rhs.v[0]; \
		lhs[1] += rhs.v[1]; \
		lhs[2] += rhs.v[2];


void OpenvrDeviceManipulationInfo::handleNewDevicePose(vr::IVRServerDriverHost* driver, _DetourTrackedDevicePoseUpdated_t origFunc, uint32_t& unWhichDevice, const vr::DriverPose_t& pose) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (m_deviceMode == 1) { // fake disconnect mode
		if (!_disconnectedMsgSend) {
			vr::DriverPose_t newPose = pose;
			newPose.poseIsValid = false;
			newPose.deviceIsConnected = false;
			newPose.result = vr::TrackingResult_Uninitialized;
			_disconnectedMsgSend = true;
			origFunc(driver, unWhichDevice, newPose, sizeof(vr::DriverPose_t));
		}
	} else if (m_deviceMode == 3 && !m_redirectSuspended) { // redirect target
		//nop
	} else if (m_deviceMode == 5) { // motion compensation mode
		auto serverDriver = CServerDriver::getInstance();
		if (serverDriver) {
			if (pose.poseIsValid && pose.result == vr::TrackingResult_Running_OK) {
				if (!serverDriver->_isMotionCompensationZeroPoseValid()) {
					serverDriver->_setMotionCompensationZeroPose(pose);
				} else {
					serverDriver->_updateMotionCompensationRefPose(pose);
				}
			}
		}
		if (!_disconnectedMsgSend) {
			vr::DriverPose_t newPose = pose;
			newPose.poseIsValid = false;
			newPose.deviceIsConnected = false;
			newPose.result = vr::TrackingResult_Uninitialized;
			_disconnectedMsgSend = true;
			origFunc(driver, unWhichDevice, newPose, sizeof(vr::DriverPose_t));
		}
	} else {
		vr::DriverPose_t newPose = pose;
		if (m_offsetsEnabled) {
			if (m_worldFromDriverRotationOffset.w != 1.0 || m_worldFromDriverRotationOffset.x != 0.0
					|| m_worldFromDriverRotationOffset.y != 0.0 || m_worldFromDriverRotationOffset.z != 0.0) {
				newPose.qWorldFromDriverRotation = m_worldFromDriverRotationOffset * newPose.qWorldFromDriverRotation;
			}
			if (m_worldFromDriverTranslationOffset.v[0] != 0.0 || m_worldFromDriverTranslationOffset.v[1] != 0.0 || m_worldFromDriverTranslationOffset.v[2] != 0.0) {
				VECTOR_ADD(newPose.vecWorldFromDriverTranslation, m_worldFromDriverTranslationOffset);
			}
			if (m_driverFromHeadRotationOffset.w != 1.0 || m_driverFromHeadRotationOffset.x != 0.0
					|| m_driverFromHeadRotationOffset.y != 0.0 || m_driverFromHeadRotationOffset.z != 0.0) {
				newPose.qDriverFromHeadRotation = m_driverFromHeadRotationOffset * newPose.qDriverFromHeadRotation;
			}
			if (m_driverFromHeadTranslationOffset.v[0] != 0.0 || m_driverFromHeadTranslationOffset.v[1] != 0.0 || m_driverFromHeadTranslationOffset.v[2] != 0.0) {
				VECTOR_ADD(newPose.vecDriverFromHeadTranslation, m_driverFromHeadTranslationOffset);
			}
			if (m_deviceRotationOffset.w != 1.0 || m_deviceRotationOffset.x != 0.0
					|| m_deviceRotationOffset.y != 0.0 || m_deviceRotationOffset.z != 0.0) {
				newPose.qRotation = m_deviceRotationOffset * newPose.qRotation;
			}
			if (m_deviceTranslationOffset.v[0] != 0.0 || m_deviceTranslationOffset.v[1] != 0.0 || m_deviceTranslationOffset.v[2] != 0.0) {
				VECTOR_ADD(newPose.vecPosition, m_deviceTranslationOffset);
			}
		}
		auto serverDriver = CServerDriver::getInstance();
		if (serverDriver) {
			serverDriver->_applyMotionCompensation(newPose, this);
		}
		if (m_deviceMode == 2 && !m_redirectSuspended) { // redirect source
			if (!_disconnectedMsgSend) {
				vr::DriverPose_t newPose2 = pose;
				newPose2.poseIsValid = false;
				newPose2.deviceIsConnected = false;
				newPose2.result = vr::TrackingResult_Uninitialized;
				_disconnectedMsgSend = true;
				origFunc(driver, unWhichDevice, newPose2, sizeof(vr::DriverPose_t));
			}
			origFunc(m_redirectRef->driverHost(), m_redirectRef->openvrId(), newPose, sizeof(vr::DriverPose_t));
		} else if (m_deviceMode == 4) { // swap mode
			origFunc(m_redirectRef->driverHost(), m_redirectRef->openvrId(), newPose, sizeof(vr::DriverPose_t));
		} else {
			origFunc(driver, unWhichDevice, newPose, sizeof(vr::DriverPose_t));
		}
	}
}


void OpenvrDeviceManipulationInfo::handleButtonEvent(vr::IVRServerDriverHost* driver, void* origFunc, uint32_t& unWhichDevice, ButtonEventType eventType, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (eButtonId == vr::k_EButton_System && (m_deviceMode == 2 || m_deviceMode == 3)) {
		if (eventType == ButtonEventType::ButtonUnpressed) {
			m_redirectSuspended = !m_redirectSuspended;
			_disconnectedMsgSend = false;
			m_redirectRef->m_redirectSuspended = m_redirectSuspended;
			m_redirectRef->_disconnectedMsgSend = false;
		}
	} else if (m_deviceMode == 1 || (m_deviceMode == 3 && !m_redirectSuspended) || m_deviceMode == 5) {
		//nop
	} else {
		vr::EVRButtonId button = eButtonId;
		getButtonMapping(eButtonId, button);
		if (m_deviceMode == 0 || ((m_deviceMode == 3 || m_deviceMode == 2) && m_redirectSuspended)) {
			((_DetourTrackedDeviceButtonPressed_t)origFunc)(driver, unWhichDevice, button, eventTimeOffset);
		} else if ((m_deviceMode == 2 && !m_redirectSuspended) || m_deviceMode == 4) {
			((_DetourTrackedDeviceButtonPressed_t)origFunc)(driver, m_redirectRef->openvrId(), button, eventTimeOffset);
		}
	}
}

void OpenvrDeviceManipulationInfo::handleAxisEvent(vr::IVRServerDriverHost* driver, _DetourTrackedDeviceAxisUpdated_t origFunc, uint32_t& unWhichDevice, uint32_t unWhichAxis, const vr::VRControllerAxis_t& axisState) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (m_deviceMode == 1 || (m_deviceMode == 3 && !m_redirectSuspended) || m_deviceMode == 5) {
		//nop
	} else {
		if (m_deviceMode == 0 || ((m_deviceMode == 3 || m_deviceMode == 2) && m_redirectSuspended)) {
			origFunc(driver, unWhichDevice, unWhichAxis, axisState);
		} else if ((m_deviceMode == 2 && !m_redirectSuspended) || m_deviceMode == 4) {
			origFunc(driver, m_redirectRef->openvrId(), unWhichAxis, axisState);
		}
	}
}


bool OpenvrDeviceManipulationInfo::triggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds, bool directMode) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (m_controllerComponent) {
		if (directMode) {
			return m_triggerHapticPulseFunc(m_controllerComponent, unAxisId, usPulseDurationMicroseconds);
		} else if ((m_deviceMode == 3 && !m_redirectSuspended) || m_deviceMode == 4) {
			m_redirectRef->triggerHapticPulse(unAxisId, usPulseDurationMicroseconds, true);
		} else  if (m_deviceMode == 0 || ((m_deviceMode == 3 || m_deviceMode == 2) && m_redirectSuspended)) {
			return m_triggerHapticPulseFunc(m_controllerComponent, unAxisId, usPulseDurationMicroseconds);
		}
	}
	return true;
}

bool OpenvrDeviceManipulationInfo::getButtonMapping(vr::EVRButtonId button, vr::EVRButtonId& mappedButton) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (m_enableButtonMapping) {
		auto i = m_buttonMapping.find(button);
		if (i != m_buttonMapping.end()) {
			mappedButton = i->second;
			return true;
		}
	}
	return false;
}

void OpenvrDeviceManipulationInfo::addButtonMapping(vr::EVRButtonId button, vr::EVRButtonId mappedButton) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	m_buttonMapping[button] = mappedButton;
}

void OpenvrDeviceManipulationInfo::eraseButtonMapping(vr::EVRButtonId button) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	m_buttonMapping.erase(button);
}

void OpenvrDeviceManipulationInfo::eraseAllButtonMappings() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	m_buttonMapping.clear();
}

int OpenvrDeviceManipulationInfo::setDefaultMode() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	auto res = _disableOldMode(0);
	if (res == 0) {
		m_deviceMode = 0;
	}
	return 0; 
}

int OpenvrDeviceManipulationInfo::setRedirectMode(bool target, OpenvrDeviceManipulationInfo* ref) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	auto res = _disableOldMode(target ? 3 : 2);
	if (res == 0) {
		m_redirectSuspended = false;
		m_redirectRef = ref;
		if (target) {
			m_deviceMode = 3;
		} else {
			m_deviceMode = 2;
		}
	}
	return 0; 
}

int OpenvrDeviceManipulationInfo::setSwapMode(OpenvrDeviceManipulationInfo* ref) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	auto res = _disableOldMode(4);
	if (res == 0) {
		m_redirectRef = ref;
		m_deviceMode = 4;
	}
	return 0;
}

int OpenvrDeviceManipulationInfo::setMotionCompensationMode() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	auto res = _disableOldMode(5);
	auto serverDriver = CServerDriver::getInstance();
	if (res == 0 && serverDriver) {
		_disconnectedMsgSend = false;
		serverDriver->enableMotionCompensation(true);
		m_deviceMode = 5;
	}
	return 0;
}

int OpenvrDeviceManipulationInfo::setFakeDisconnectedMode() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	auto res = _disableOldMode(1);
	if (res == 0) {
		_disconnectedMsgSend = false;
		m_deviceMode = 1;
	}
	return 0;
}

int OpenvrDeviceManipulationInfo::_disableOldMode(int newMode) {
	if (m_deviceMode != newMode) {
		if (m_deviceMode == 5) {
			auto serverDriver = CServerDriver::getInstance();
			if (serverDriver) {
				serverDriver->enableMotionCompensation(false);
			}
		} else if (m_deviceMode == 3 || m_deviceMode == 2 || m_deviceMode == 4) {
			m_redirectRef->m_deviceMode = 0;
		}
		if (newMode == 5) {
			auto serverDriver = CServerDriver::getInstance();
			if (serverDriver) {
				serverDriver->disableMotionCompensationOnAllDevices();
				serverDriver->enableMotionCompensation(false);
			}
		} 
	}
	return 0;
}


void OpenvrDeviceManipulationInfo::setControllerComponent(vr::IVRControllerComponent* component, _DetourTriggerHapticPulse_t triggerHapticPulse) {
	m_controllerComponent = component;
	m_triggerHapticPulseFunc = triggerHapticPulse;
}

} // end namespace driver
} // end namespace vrinputemulator
