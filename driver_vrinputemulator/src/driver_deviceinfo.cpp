
#include "stdafx.h"
#include "driver_vrinputemulator.h"
#include <openvr_math.h>


namespace vrinputemulator {
namespace driver {


#define VECTOR_ADD(lhs, rhs) \
		lhs[0] += rhs.v[0]; \
		lhs[1] += rhs.v[1]; \
		lhs[2] += rhs.v[2];




void OpenvrDeviceManipulationInfo::setDigitalInputRemapping(uint32_t buttonId, const DigitalInputRemapping& remapping) {
	auto& entry = m_digitalInputRemapping[buttonId];
	entry.remapping = remapping;
}


DigitalInputRemapping OpenvrDeviceManipulationInfo::getDigitalInputRemapping(uint32_t buttonId) {
	auto it = m_digitalInputRemapping.find(buttonId);
	if (it != m_digitalInputRemapping.end()) {
		return it->second.remapping;
	} else {
		return DigitalInputRemapping();
	}
}


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
					serverDriver->sendReplySetMotionCompensationMode(true);
				} else {
					serverDriver->_updateMotionCompensationRefPose(pose);
				}
			} else {
				if (!serverDriver->_isMotionCompensationZeroPoseValid()) {
					serverDriver->sendReplySetMotionCompensationMode(false);
				}
			}
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
			if (!serverDriver->_applyMotionCompensation(newPose, this)) {
				return;
			}
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


void OpenvrDeviceManipulationInfo::handleButtonEvent(vr::IVRServerDriverHost* driver, uint32_t& unWhichDevice, ButtonEventType eventType, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	auto buttonIt = m_digitalInputRemapping.find(eButtonId);
	if (buttonIt != m_digitalInputRemapping.end()) {
		auto& buttonInfo = buttonIt->second;
		if (buttonInfo.remapping.touchAsClick) {
			switch (eventType) {
			case vrinputemulator::ButtonEventType::ButtonPressed:
			case vrinputemulator::ButtonEventType::ButtonUnpressed:
				return;
			case vrinputemulator::ButtonEventType::ButtonTouched: {
				eventType = vrinputemulator::ButtonEventType::ButtonPressed;
			} break;
			case vrinputemulator::ButtonEventType::ButtonUntouched: {
				eventType = vrinputemulator::ButtonEventType::ButtonUnpressed;
			} break;
			default: {
			} break;
			}
		}
		if (eventType == ButtonEventType::ButtonTouched || eventType == ButtonEventType::ButtonUntouched) {
			if (!buttonInfo.remapping.doublePressEnabled && !buttonInfo.remapping.longPressEnabled) {
				sendDigitalBinding(buttonInfo.remapping.binding, buttonInfo.bindings[0], unWhichDevice, eventType, eButtonId, eventTimeOffset);
			}
		} else if (eventType == ButtonEventType::ButtonPressed || eventType == ButtonEventType::ButtonUnpressed) {
			switch (buttonInfo.state) {
			case 0: {
				if (!buttonInfo.remapping.doublePressEnabled && !buttonInfo.remapping.longPressEnabled) {
					sendDigitalBinding(buttonInfo.remapping.binding, buttonInfo.bindings[0], unWhichDevice, eventType, eButtonId, eventTimeOffset);
					LOG(INFO) << "buttonInfo.state = 0: sendDigitalBinding";
				} else if (eventType == ButtonEventType::ButtonPressed) {
					if (buttonInfo.remapping.longPressEnabled) {
						buttonInfo.timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(buttonInfo.remapping.longPressThreshold);
					}
					buttonInfo.state = 1;
					LOG(INFO) << "buttonInfo.state = 0: => 1";
				}
			} break;
			case 1: {
				if (eventType == ButtonEventType::ButtonUnpressed) {
					if (buttonInfo.remapping.doublePressEnabled) {
						buttonInfo.timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(buttonInfo.remapping.doublePressThreshold);
						buttonInfo.state = 3;
						LOG(INFO) << "buttonInfo.state = 1: => 3";
					} else {
						sendDigitalBinding(buttonInfo.remapping.binding, buttonInfo.bindings[0], m_openvrId, ButtonEventType::ButtonPressed, eButtonId, 0.0);
						buttonInfo.timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(100);
						buttonInfo.state = 4;
						LOG(INFO) << "buttonInfo.state = 1: => 4";
					}
				}
			} break;
			case 2: {
				if (eventType == ButtonEventType::ButtonUnpressed) {
					sendDigitalBinding(buttonInfo.remapping.longPressBinding, buttonInfo.bindings[1], unWhichDevice, eventType, eButtonId, eventTimeOffset);
					buttonInfo.state = 0;
					LOG(INFO) << "buttonInfo.state = 2: sendDigitalBinding, => 0";
				}
			} break;
			case 3: {
				if (eventType == ButtonEventType::ButtonPressed) {
					sendDigitalBinding(buttonInfo.remapping.doublePressBinding, buttonInfo.bindings[2], unWhichDevice, eventType, eButtonId, eventTimeOffset);
					buttonInfo.state = 5;
					LOG(INFO) << "buttonInfo.state = 3: sendDigitalBinding, => 5";
				}
			} break;
			case 5: {
				if (eventType == ButtonEventType::ButtonUnpressed) {
					sendDigitalBinding(buttonInfo.remapping.doublePressBinding, buttonInfo.bindings[2], unWhichDevice, eventType, eButtonId, eventTimeOffset);
					buttonInfo.state = 0;
					LOG(INFO) << "buttonInfo.state = 5: sendDigitalBinding, => 0";
				}
			} break;
			default: {
			} break;
			}
		}
	} else {
		if (m_deviceMode == 1 || (m_deviceMode == 3 && !m_redirectSuspended) /*|| m_deviceMode == 5*/) {
			//nop
		} else {
			sendButtonEvent(unWhichDevice, eventType, eButtonId, eventTimeOffset);
		}
	}
}


void OpenvrDeviceManipulationInfo::suspendRedirectMode() {
	if (m_deviceMode == 2 || m_deviceMode == 3) {
		m_redirectSuspended = !m_redirectSuspended;
		_disconnectedMsgSend = false;
		m_redirectRef->m_redirectSuspended = m_redirectSuspended;
		m_redirectRef->_disconnectedMsgSend = false;
	}
}


void OpenvrDeviceManipulationInfo::RunFrame() {
	for (auto& r : m_digitalInputRemapping) {
		switch (r.second.state) {
		case 1: {
			if (r.second.remapping.longPressEnabled) {
				auto now = std::chrono::system_clock::now();
				if (r.second.timeout <= now) {
					sendDigitalBinding(r.second.remapping.longPressBinding, r.second.bindings[1], m_openvrId, ButtonEventType::ButtonPressed, (vr::EVRButtonId)r.first, 0.0);
					r.second.state = 2;
					LOG(INFO) << "buttonInfo.state = 1: sendDigitalBinding, => 2";
				}
			}
		} break;
		case 3: {
			auto now = std::chrono::system_clock::now();
			if (r.second.timeout <= now) {
				sendDigitalBinding(r.second.remapping.binding, r.second.bindings[0], m_openvrId, ButtonEventType::ButtonPressed, (vr::EVRButtonId)r.first, 0.0);
				r.second.timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(100);
				r.second.state = 4;
				LOG(INFO) << "buttonInfo.state = 3: sendDigitalBinding, => 4";
			}
		} break;
		case 4: {
			auto now = std::chrono::system_clock::now();
			if (r.second.timeout <= now) {
				sendDigitalBinding(r.second.remapping.binding, r.second.bindings[0], m_openvrId, ButtonEventType::ButtonUnpressed, (vr::EVRButtonId)r.first, 0.0);
				r.second.state = 0;
				LOG(INFO) << "buttonInfo.state = 4: sendDigitalBinding, => 0";
			}
		} break;
		default:
			break;
		}
		RunFrameDigitalBinding(r.second.remapping.binding, r.second.bindings[0]);
		RunFrameDigitalBinding(r.second.remapping.longPressBinding, r.second.bindings[1]);
		RunFrameDigitalBinding(r.second.remapping.doublePressBinding, r.second.bindings[2]);
	}
}


void OpenvrDeviceManipulationInfo::RunFrameDigitalBinding(vrinputemulator::DigitalBinding& binding, OpenvrDeviceManipulationInfo::DigitalInputRemappingInfo::BindingInfo& bindingInfo) {
	switch (bindingInfo.state) {
		case 1: {
			if (binding.toggleEnabled) {
				auto now = std::chrono::system_clock::now();
				if (bindingInfo.timeout <= now) {
					bindingInfo.state = 2;
				}
			}
		} break;
		default: {
		} break;
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



void OpenvrDeviceManipulationInfo::sendDigitalBinding(vrinputemulator::DigitalBinding& binding, OpenvrDeviceManipulationInfo::DigitalInputRemappingInfo::BindingInfo& bindingInfo,
		uint32_t unWhichDevice, ButtonEventType eventType, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	if (binding.type == DigitalBindingType::NoRemapping) {
		sendButtonEvent(unWhichDevice, eventType, eButtonId, eventTimeOffset, false, &bindingInfo);
	} else if (binding.type == DigitalBindingType::Disabled) {
		// nop
	} else {
		bool sendEvent = false;
		if (eventType == ButtonEventType::ButtonPressed || eventType == ButtonEventType::ButtonUnpressed) {
			switch (bindingInfo.state) {
				case 0:  {
					if (eventType == ButtonEventType::ButtonPressed) {
						sendEvent = true;
						if (binding.toggleEnabled) {
							bindingInfo.timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(binding.toggleDelay);
						}
						bindingInfo.state = 1;
					}
				} break;
				case 1: {
					if (eventType == ButtonEventType::ButtonUnpressed) {
						sendEvent = true;
						bindingInfo.state = 0;
					}
				} break;
				case 2: {
					if (eventType == ButtonEventType::ButtonPressed) {
						bindingInfo.state = 3;
					}
				} break;
				case 3: {
					if (eventType == ButtonEventType::ButtonUnpressed) {
						sendEvent = true;
						bindingInfo.state = 0;
					}
				} break;
				default: {
				} break;
			}
		} else if (eventType == ButtonEventType::ButtonTouched || eventType == ButtonEventType::ButtonUntouched) {
			sendEvent = true;
		}
		if (sendEvent) {
			switch (binding.type) {
				case DigitalBindingType::OpenVR: {
					if (m_deviceMode == 1 || (m_deviceMode == 3 && !m_redirectSuspended) /*|| m_deviceMode == 5*/) {
						//nop
					} else {
						vr::EVRButtonId button = (vr::EVRButtonId)binding.binding.openvr.buttonId;
						uint32_t deviceId = binding.binding.openvr.controllerId;
						if (deviceId >= 999) {
							deviceId = m_openvrId;
						}
						sendButtonEvent(deviceId, eventType, button, eventTimeOffset, false, &bindingInfo);
					}
				} break;
				case DigitalBindingType::Keyboard: {
					if (m_deviceMode == 1 || (m_deviceMode == 3 && !m_redirectSuspended) /*|| m_deviceMode == 5*/) {
						//nop
					} else {
						sendKeyboardEvent(eventType, binding.binding.keyboard.shiftPressed, binding.binding.keyboard.ctrlPressed, 
							binding.binding.keyboard.altPressed, (WORD)binding.binding.keyboard.keyCode, &bindingInfo);
					}
				} break;
				case DigitalBindingType::SuspendRedirectMode: {
					if (eventType == ButtonEventType::ButtonPressed) {
						suspendRedirectMode();
					}
				} break;
				default: {
				} break;
			}
		}
	}
}

void OpenvrDeviceManipulationInfo::sendButtonEvent(uint32_t& unWhichDevice, ButtonEventType eventType, vr::EVRButtonId eButtonId, double eventTimeOffset, bool directMode, DigitalInputRemappingInfo::BindingInfo* binding) {
	if (!directMode) {
		if (unWhichDevice == m_openvrId && ((m_deviceMode == 2 && !m_redirectSuspended) || m_deviceMode == 4)) {
			unWhichDevice = m_redirectRef->openvrId();
		}
		if (unWhichDevice != m_openvrId) {
			auto deviceInfo = CServerDriver::getDeviceInfo(unWhichDevice);
			if (deviceInfo) {
				return deviceInfo->sendButtonEvent(unWhichDevice, eventType, eButtonId, eventTimeOffset, true, binding);
			}
		}
	}
	switch (eventType) {
		case ButtonEventType::ButtonPressed: {
			if (binding) {
				if (!binding->touchedState) {
					CServerDriver::openvrTrackedDeviceButtonTouched(m_driverHost, unWhichDevice, eButtonId, eventTimeOffset);
					binding->touchedState = true;
					binding->touchedAutoset = true;
				}
				if (!binding->pressedState) {
					CServerDriver::openvrTrackedDeviceButtonPressed(m_driverHost, unWhichDevice, eButtonId, eventTimeOffset);
					binding->pressedState = true;
				}
			} else {
				CServerDriver::openvrTrackedDeviceButtonPressed(m_driverHost, unWhichDevice, eButtonId, eventTimeOffset);
			}
		} break;
		case ButtonEventType::ButtonUnpressed: {
			if (binding) {
				if (binding->pressedState) {
					CServerDriver::openvrTrackedDeviceButtonUnpressed(m_driverHost, unWhichDevice, eButtonId, eventTimeOffset);
					binding->pressedState = false;
				}
				if (binding->touchedState && binding->touchedAutoset) {
					CServerDriver::openvrTrackedDeviceButtonUntouched(m_driverHost, unWhichDevice, eButtonId, eventTimeOffset);
					binding->touchedState = false;
					binding->touchedAutoset = false;
				}
			} else {
				CServerDriver::openvrTrackedDeviceButtonUnpressed(m_driverHost, unWhichDevice, eButtonId, eventTimeOffset);
			}
		} break;
		case ButtonEventType::ButtonTouched: {
			if (binding) {
				if (!binding->touchedState) {
					CServerDriver::openvrTrackedDeviceButtonTouched(m_driverHost, unWhichDevice, eButtonId, eventTimeOffset);
					binding->touchedState = true;
					binding->touchedAutoset = false;
				}
			} else {
				CServerDriver::openvrTrackedDeviceButtonTouched(m_driverHost, unWhichDevice, eButtonId, eventTimeOffset);
			}
		} break;
		case ButtonEventType::ButtonUntouched: {
			if (binding) {
				if (binding->touchedState) {
					if (binding->pressedState) {
						binding->touchedAutoset = true;
					} else {
						CServerDriver::openvrTrackedDeviceButtonUntouched(m_driverHost, unWhichDevice, eButtonId, eventTimeOffset);
						binding->touchedState = false;
						binding->touchedAutoset = false;
					}
				}
			} else {
				CServerDriver::openvrTrackedDeviceButtonUntouched(m_driverHost, unWhichDevice, eButtonId, eventTimeOffset);
			}
		} break;
		default: {
		} break;
	}
}

void OpenvrDeviceManipulationInfo::sendKeyboardEvent(ButtonEventType eventType, bool shiftPressed, bool ctrlPressed, bool altPressed, WORD keyCode, DigitalInputRemappingInfo::BindingInfo* binding) {
	if ( (eventType == ButtonEventType::ButtonPressed && (!binding || !binding->pressedState)) 
			|| (eventType == ButtonEventType::ButtonUnpressed && (!binding || binding->pressedState)) ) {
		INPUT ips[4];
		memset(ips, 0, sizeof(ips));
		WORD flags = 0;
		if (eventType == ButtonEventType::ButtonUnpressed) {
			flags = KEYEVENTF_KEYUP;
		}
		unsigned ipCount = 0;
		if (eventType == ButtonEventType::ButtonUnpressed && keyCode) {
			ips[ipCount].type = INPUT_KEYBOARD;
			ips[ipCount].ki.wVk = keyCode;
			ips[ipCount].ki.dwFlags = flags;
			ipCount++;
		}
		if (shiftPressed) {
			ips[ipCount].type = INPUT_KEYBOARD;
			ips[ipCount].ki.wVk = VK_SHIFT;
			ips[ipCount].ki.dwFlags = flags;
			ipCount++;
		}
		if (ctrlPressed) {
			ips[ipCount].type = INPUT_KEYBOARD;
			ips[ipCount].ki.wVk = VK_CONTROL;
			ips[ipCount].ki.dwFlags = flags;
			ipCount++;
		}
		if (altPressed) {
			ips[ipCount].type = INPUT_KEYBOARD;
			ips[ipCount].ki.wVk = VK_MENU;
			ips[ipCount].ki.dwFlags = flags;
			ipCount++;
		}
		if (eventType == ButtonEventType::ButtonPressed && keyCode) {
			ips[ipCount].type = INPUT_KEYBOARD;
			ips[ipCount].ki.wVk = keyCode;
			ips[ipCount].ki.dwFlags = flags;
			ipCount++;
		}
		SendInput(4, ips, sizeof(INPUT));
		if (binding) {
			if (eventType == ButtonEventType::ButtonPressed) {
				binding->pressedState = true;
			} else if (eventType == ButtonEventType::ButtonUnpressed) {
				binding->pressedState = false;
			}
		}
	}
}


/*bool OpenvrDeviceManipulationInfo::getButtonMapping(vr::EVRButtonId button, vr::EVRButtonId& mappedButton) {
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
}*/

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
		serverDriver->enableMotionCompensation(true);
		serverDriver->setMotionCompensationRefDevice(this);
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
				serverDriver->setMotionCompensationRefDevice(nullptr);
			}
		} else if (m_deviceMode == 3 || m_deviceMode == 2 || m_deviceMode == 4) {
			m_redirectRef->m_deviceMode = 0;
		}
		if (newMode == 5) {
			auto serverDriver = CServerDriver::getInstance();
			if (serverDriver) {
				serverDriver->_disableMotionCompensationOnAllDevices();
				serverDriver->enableMotionCompensation(false);
				serverDriver->setMotionCompensationRefDevice(nullptr);
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
