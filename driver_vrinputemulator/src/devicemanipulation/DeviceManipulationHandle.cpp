#include "DeviceManipulationHandle.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include "../driver/ServerDriver.h"
#include "../hooks/IVRServerDriverHost004Hooks.h"
#include "../hooks/IVRServerDriverHost005Hooks.h"
#include "../hooks/IVRControllerComponent001Hooks.h"
#include "../hooks/IVRDriverInput001Hooks.h"

#undef WIN32_LEAN_AND_MEAN
#undef NOSOUND
#include <Windows.h>
// According to windows documentation mmsystem.h should be automatically included with Windows.h when WIN32_LEAN_AND_MEAN and NOSOUND are not defined
// But it doesn't work so I have to include it manually
#include <mmsystem.h>


namespace vrinputemulator {
namespace driver {


#define VECTOR_ADD(lhs, rhs) \
		lhs[0] += rhs.v[0]; \
		lhs[1] += rhs.v[1]; \
		lhs[2] += rhs.v[2];


bool DeviceManipulationHandle::touchpadEmulationEnabledFlag = true;


DeviceManipulationHandle::DeviceManipulationHandle(const char* serial, vr::ETrackedDeviceClass eDeviceClass, void* driverPtr, void* driverHostPtr, int driverInterfaceVersion)
		: m_isValid(true), m_parent(ServerDriver::getInstance()), m_motionCompensationManager(m_parent->motionCompensation()), m_deviceDriverPtr(driverPtr), m_deviceDriverHostPtr(driverHostPtr),
		m_deviceDriverInterfaceVersion(driverInterfaceVersion), m_eDeviceClass(eDeviceClass), m_serialNumber(serial) {
	memset(_AxisIdToComponentHandleMap, 0, sizeof(_AxisIdToComponentHandleMap));
}


void DeviceManipulationHandle::setDigitalInputRemapping(uint32_t buttonId, const DigitalInputRemapping& remapping) {
	if (remapping.valid) {
		m_digitalInputRemapping[buttonId].remapping = remapping;
	} else {
		auto it = m_digitalInputRemapping.find(buttonId);
		if (it != m_digitalInputRemapping.end()) {
			m_digitalInputRemapping.erase(it);
		}
	}
}


DigitalInputRemapping DeviceManipulationHandle::getDigitalInputRemapping(uint32_t buttonId) {
	auto it = m_digitalInputRemapping.find(buttonId);
	if (it != m_digitalInputRemapping.end()) {
		return it->second.remapping;
	} else {
		return DigitalInputRemapping();
	}
}


void DeviceManipulationHandle::setAnalogInputRemapping(uint32_t axisId, const AnalogInputRemapping& remapping) {
	if (axisId < 5) {
		m_analogInputRemapping[axisId].remapping = remapping;
	}
}


AnalogInputRemapping DeviceManipulationHandle::getAnalogInputRemapping(uint32_t axisId) {
	if (axisId < 5) {
		return m_analogInputRemapping[axisId].remapping;
	} else {
		return AnalogInputRemapping();
	}
}

bool DeviceManipulationHandle::handlePoseUpdate(uint32_t& unWhichDevice, vr::DriverPose_t& newPose, uint32_t unPoseStructSize) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);

	if (m_deviceMode == 1) { // fake disconnect mode
		if (!_disconnectedMsgSend) {
			newPose.poseIsValid = false;
			newPose.deviceIsConnected = false;
			newPose.result = vr::TrackingResult_Uninitialized;
			_disconnectedMsgSend = true;
			return true;
		} else {
			return false;
		}

	} else if (m_deviceMode == 3 && !m_redirectSuspended) { // redirect target
		return false;

	} else if (m_deviceMode == 5) { // motion compensation mode
		auto serverDriver = ServerDriver::getInstance();
		if (serverDriver) {
			if (newPose.poseIsValid && newPose.result == vr::TrackingResult_Running_OK) {
				m_motionCompensationManager._setMotionCompensationStatus(MotionCompensationStatus::Running);
				if (!m_motionCompensationManager._isMotionCompensationZeroPoseValid()) {
					m_motionCompensationManager._setMotionCompensationZeroPose(newPose);
					serverDriver->sendReplySetMotionCompensationMode(true);
				} else {
					m_motionCompensationManager._updateMotionCompensationRefPose(newPose);
				}
			} else {
				if (!m_motionCompensationManager._isMotionCompensationZeroPoseValid()) {
					setDefaultMode();
					serverDriver->sendReplySetMotionCompensationMode(false);
				} else {
					m_motionCompensationManager._setMotionCompensationStatus(MotionCompensationStatus::MotionRefNotTracking);
				}
			}
		}
		return true;

	} else {
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
		
		m_motionCompensationManager._applyMotionCompensation(newPose, this);
		
		if (m_deviceMode == 2 && !m_redirectSuspended) { // redirect source
			m_redirectRef->ll_sendPoseUpdate(newPose);
			if (!_disconnectedMsgSend) {
				newPose.poseIsValid = false;
				newPose.deviceIsConnected = false;
				newPose.result = vr::TrackingResult_Uninitialized;
				_disconnectedMsgSend = true;
			} else {
				return false;
			}
		} else if (m_deviceMode == 4) { // swap mode
			unWhichDevice = m_redirectRef->openvrId();
		}
		return true;
	}
}


void DeviceManipulationHandle::ll_sendPoseUpdate(const vr::DriverPose_t& newPose) {
	if (m_deviceDriverInterfaceVersion == 4) {
		IVRServerDriverHost004Hooks::trackedDevicePoseUpdatedOrig(m_deviceDriverHostPtr, m_openvrId, newPose, sizeof(vr::DriverPose_t));
	} else if (m_deviceDriverInterfaceVersion == 5) {
		IVRServerDriverHost005Hooks::trackedDevicePoseUpdatedOrig(m_deviceDriverHostPtr, m_openvrId, newPose, sizeof(vr::DriverPose_t));
	}
}


void DeviceManipulationHandle::ll_sendButtonEvent(ButtonEventType eventType, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	if (m_deviceDriverInterfaceVersion == 4) {
		switch (eventType) {
		case vrinputemulator::ButtonEventType::ButtonPressed:
			IVRServerDriverHost004Hooks::trackedDeviceButtonPressedOrig(m_deviceDriverHostPtr, m_openvrId, eButtonId, eventTimeOffset);
			break;
		case vrinputemulator::ButtonEventType::ButtonUnpressed:
			IVRServerDriverHost004Hooks::trackedDeviceButtonUnpressedOrig(m_deviceDriverHostPtr, m_openvrId, eButtonId, eventTimeOffset);
			break;
		case vrinputemulator::ButtonEventType::ButtonTouched:
			IVRServerDriverHost004Hooks::trackedDeviceButtonTouchedOrig(m_deviceDriverHostPtr, m_openvrId, eButtonId, eventTimeOffset);
			break;
		case vrinputemulator::ButtonEventType::ButtonUntouched:
			IVRServerDriverHost004Hooks::trackedDeviceButtonUntouchedOrig(m_deviceDriverHostPtr, m_openvrId, eButtonId, eventTimeOffset);
			break;
		default:
			break;
		}
	} else if (m_deviceDriverInterfaceVersion == 5) {
		auto it = _ButtonIdToComponentHandleMap.find(eButtonId);
		if (it != _ButtonIdToComponentHandleMap.end()) {
			uint64_t componentHandle = 0;
			bool newValue = false;
			switch (eventType)	{
				case ButtonEventType::ButtonTouched:
					componentHandle = it->second.first;
					newValue = true;
					break;
				case ButtonEventType::ButtonUntouched:
					componentHandle = it->second.first;
					newValue = false;
					break;
				case ButtonEventType::ButtonPressed:
					componentHandle = it->second.second;
					newValue = true;
					break;
				case ButtonEventType::ButtonUnpressed:
					componentHandle = it->second.second;
					newValue = false;
					break;
				default:
					break;
			}
			if (componentHandle != 0) {
				IVRDriverInput001Hooks::updateBooleanComponentOrig(m_driverInputPtr, componentHandle, newValue, eventTimeOffset);
			}
		} else {
			LOG(WARNING) << "Device " << m_openvrId << ": No mapping from button id " << eButtonId << " to input component.";
		}
	}
}


void DeviceManipulationHandle::ll_sendAxisEvent(uint32_t unWhichAxis, const vr::VRControllerAxis_t& axisState) {
	if (m_deviceDriverInterfaceVersion == 4) {
		IVRServerDriverHost004Hooks::trackedDeviceAxisUpdatedOrig(m_deviceDriverHostPtr, m_openvrId, unWhichAxis, axisState);
	} else if (m_deviceDriverInterfaceVersion == 5) {
		if (unWhichAxis < 5) {
			if (_AxisIdToComponentHandleMap[unWhichAxis].first == 0 && _AxisIdToComponentHandleMap[unWhichAxis].second == 0) {
				LOG(WARNING) << "Device " << m_openvrId << ": No mapping from axis id " << unWhichAxis << " to input component.";
			} else {
				if (_AxisIdToComponentHandleMap[unWhichAxis].first != 0) {
					sendScalarComponentUpdate(m_openvrId, unWhichAxis, 0, axisState.x, 0.0);
				}
				if (_AxisIdToComponentHandleMap[unWhichAxis].second != 0) {
					sendScalarComponentUpdate(m_openvrId, unWhichAxis, 1, axisState.y, 0.0);
				} 
			}
		}
	}
}


bool DeviceManipulationHandle::ll_triggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds) {
	if (m_deviceDriverInterfaceVersion == 4 && m_controllerComponentHooks) {
		return std::static_pointer_cast<IVRControllerComponent001Hooks>(m_controllerComponentHooks)->triggerHapticPulseOrig(unAxisId, usPulseDurationMicroseconds);
	} else if (m_deviceDriverInterfaceVersion == 5) {
		
		struct VREvent_HapticVibration_t {
			uint64_t containerHandle; // property container handle of the device with the haptic component
			uint64_t componentHandle; // Which haptic component needs to vibrate
			float fDurationSeconds;
			float fFrequency;
			float fAmplitude;
		};

		vr::VREvent_t* hapticEvent = (vr::VREvent_t*)malloc(48);
		hapticEvent->eventType = 1700; // VREvent_Input_HapticVibration = 1700
		hapticEvent->eventAgeSeconds = 0.0f;
		hapticEvent->trackedDeviceIndex = m_openvrId;
		auto eventData = reinterpret_cast<VREvent_HapticVibration_t*>(&hapticEvent->data);
		eventData->containerHandle = m_propertyContainerHandle;
		eventData->componentHandle = m_inputHapticComponentHandle;
		eventData->fDurationSeconds = ((float)usPulseDurationMicroseconds)/1E6f;
		eventData->fFrequency = 1.0f/ eventData->fDurationSeconds;
		eventData->fAmplitude = 1.0f;
		m_parent->addDriverEventForInjection(m_deviceDriverHostPtr, std::shared_ptr<void>(hapticEvent), 48);
	}
	return true;
}


bool DeviceManipulationHandle::ll_sendHapticPulseEvent(float fDurationSeconds, float fFrequency, float fAmplitude) {
	if (m_deviceDriverInterfaceVersion == 4 && m_controllerComponentHooks) {
		return std::static_pointer_cast<IVRControllerComponent001Hooks>(m_controllerComponentHooks)->triggerHapticPulseOrig(0, (uint16_t)(fDurationSeconds * 1E6));
	} else if (m_deviceDriverInterfaceVersion == 5) {

		struct VREvent_HapticVibration_t {
			uint64_t containerHandle; // property container handle of the device with the haptic component
			uint64_t componentHandle; // Which haptic component needs to vibrate
			float fDurationSeconds;
			float fFrequency;
			float fAmplitude;
		};

		vr::VREvent_t* hapticEvent = (vr::VREvent_t*)malloc(48);
		hapticEvent->eventType = 1700; // VREvent_Input_HapticVibration = 1700
		hapticEvent->eventAgeSeconds = 0.0f;
		hapticEvent->trackedDeviceIndex = m_openvrId;
		auto eventData = reinterpret_cast<VREvent_HapticVibration_t*>(&hapticEvent->data);
		eventData->containerHandle = m_propertyContainerHandle;
		eventData->componentHandle = m_inputHapticComponentHandle;
		eventData->fDurationSeconds = fDurationSeconds;
		eventData->fFrequency = fFrequency;
		eventData->fAmplitude = fAmplitude;
		m_parent->addDriverEventForInjection(m_deviceDriverHostPtr, std::shared_ptr<void>(hapticEvent), 48);
	}
	return true;
}


void DeviceManipulationHandle::ll_sendScalarComponentUpdate(vr::VRInputComponentHandle_t ulComponent, float fNewValue, double fTimeOffset) {
	if (m_deviceDriverInterfaceVersion == 5) {
		IVRDriverInput001Hooks::updateScalarComponentOrig(m_driverInputPtr, ulComponent, fNewValue, fTimeOffset);
	} else if (m_deviceDriverInterfaceVersion == 4) {
		auto it = _componentHandleToAxisIdMap.find(ulComponent);
		if (it != _componentHandleToAxisIdMap.end()) {
			std::lock_guard<std::recursive_mutex> lock(_mutex);
			unsigned unWhichAxis = it->second.first;
			unsigned unWhichAxisDim = it->second.second;
			if (unWhichAxis < 5) {
				auto& axisInfo = m_analogInputRemapping[unWhichAxis];
				if (unWhichAxisDim == 0) {
					axisInfo.binding.lastSeenAxisState.x = fNewValue;
					axisInfo.binding.lastSendAxisState.x = fNewValue;
				} else {
					axisInfo.binding.lastSeenAxisState.x = fNewValue;
					axisInfo.binding.lastSendAxisState.x = fNewValue;
				}
				IVRServerDriverHost004Hooks::trackedDeviceAxisUpdatedOrig(m_deviceDriverHostPtr, m_openvrId, unWhichAxis, axisInfo.binding.lastSendAxisState);
			}
		}
	}
}


bool DeviceManipulationHandle::handleButtonEvent(uint32_t& unWhichDevice, ButtonEventType eventType, vr::EVRButtonId& eButtonId, double& eventTimeOffset) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	auto buttonIt = m_digitalInputRemapping.find(eButtonId);
	if (buttonIt != m_digitalInputRemapping.end()) {
		auto& buttonInfo = buttonIt->second;
		if (buttonInfo.remapping.valid) {
			if (buttonInfo.remapping.touchAsClick) {
				switch (eventType) {
					case vrinputemulator::ButtonEventType::ButtonPressed:
					case vrinputemulator::ButtonEventType::ButtonUnpressed:
						return false;
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
					sendDigitalBinding(buttonInfo.remapping.binding, unWhichDevice, eventType, eButtonId, eventTimeOffset, &buttonInfo.bindings[0]);
				}
			} else if (eventType == ButtonEventType::ButtonPressed || eventType == ButtonEventType::ButtonUnpressed) {
				switch (buttonInfo.state) {
				case 0: {
					if (!buttonInfo.remapping.doublePressEnabled && !buttonInfo.remapping.longPressEnabled) {
						//LOG(INFO) << "buttonInfo.state = 0: sendDigitalBinding - EventType: " << (int)eventType;
						sendDigitalBinding(buttonInfo.remapping.binding, unWhichDevice, eventType, eButtonId, eventTimeOffset, &buttonInfo.bindings[0]);
					} else if (eventType == ButtonEventType::ButtonPressed) {
						if (buttonInfo.remapping.longPressEnabled) {
							buttonInfo.timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(buttonInfo.remapping.longPressThreshold);
						}
						buttonInfo.state = 1;
						//LOG(INFO) << "buttonInfo.state = 0: => 1";
					}
				} break;
				case 1: {
					if (eventType == ButtonEventType::ButtonUnpressed) {
						if (buttonInfo.remapping.doublePressEnabled) {
							buttonInfo.timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(buttonInfo.remapping.doublePressThreshold);
							buttonInfo.state = 3;
							//LOG(INFO) << "buttonInfo.state = 1: => 3";
						} else {
							sendDigitalBinding(buttonInfo.remapping.binding, m_openvrId, ButtonEventType::ButtonPressed, eButtonId, 0.0, &buttonInfo.bindings[0]);
							buttonInfo.timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(100);
							buttonInfo.state = 4;
							//LOG(INFO) << "buttonInfo.state = 1: => 4";
						}
					}
				} break;
				case 2: {
					if (eventType == ButtonEventType::ButtonUnpressed) {
						sendDigitalBinding(buttonInfo.remapping.longPressBinding, unWhichDevice, eventType, eButtonId, eventTimeOffset, &buttonInfo.bindings[1]);
						buttonInfo.state = 0;
						//LOG(INFO) << "buttonInfo.state = 2: sendDigitalBinding, => 0";
					}
				} break;
				case 3: {
					if (eventType == ButtonEventType::ButtonPressed) {
						sendDigitalBinding(buttonInfo.remapping.doublePressBinding, unWhichDevice, eventType, eButtonId, eventTimeOffset, &buttonInfo.bindings[2]);
						if (buttonInfo.remapping.doublePressImmediateRelease) {
							buttonInfo.timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(100);
						}
						buttonInfo.state = 5;
						//LOG(INFO) << "buttonInfo.state = 3: sendDigitalBinding, => 5";
					}
				} break;
				case 5: {
					if (eventType == ButtonEventType::ButtonUnpressed) {
						sendDigitalBinding(buttonInfo.remapping.doublePressBinding, unWhichDevice, eventType, eButtonId, eventTimeOffset, &buttonInfo.bindings[2]);
						buttonInfo.state = 0;
						//LOG(INFO) << "buttonInfo.state = 5: sendDigitalBinding, => 0";
					}
				} break;
				case 6: {
					if (eventType == ButtonEventType::ButtonUnpressed) {
						buttonInfo.state = 0;
						//LOG(INFO) << "buttonInfo.state = 6: => 0";
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
	} else {
		if (m_deviceMode == 1 || (m_deviceMode == 3 && !m_redirectSuspended) /*|| m_deviceMode == 5*/) {
			//nop
		} else {
			sendButtonEvent(unWhichDevice, eventType, eButtonId, eventTimeOffset);
		}
	}
	return false;
}


void DeviceManipulationHandle::suspendRedirectMode() {
	if (m_deviceMode == 2 || m_deviceMode == 3) {
		m_redirectSuspended = !m_redirectSuspended;
		_disconnectedMsgSend = false;
		m_redirectRef->m_redirectSuspended = m_redirectSuspended;
		m_redirectRef->_disconnectedMsgSend = false;
	}
}


void DeviceManipulationHandle::RunFrame() {
	for (auto& r : m_digitalInputRemapping) {
		switch (r.second.state) {
		case 1: {
			if (r.second.remapping.longPressEnabled) {
				auto now = std::chrono::system_clock::now();
				if (r.second.timeout <= now) {
					sendDigitalBinding(r.second.remapping.longPressBinding, m_openvrId, ButtonEventType::ButtonPressed, (vr::EVRButtonId)r.first, 0.0, &r.second.bindings[1]);
					if (r.second.remapping.longPressImmediateRelease) {
						r.second.timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(100);
					}
					r.second.state = 2;
					//LOG(INFO) << "buttonInfo.state = 1: sendDigitalBinding, => 2";
				}
			}
		} break;
		case 2: {
			if (r.second.remapping.longPressImmediateRelease) {
				auto now = std::chrono::system_clock::now();
				if (r.second.timeout <= now) {
					sendDigitalBinding(r.second.remapping.longPressBinding, m_openvrId, ButtonEventType::ButtonUnpressed, (vr::EVRButtonId)r.first, 0.0, &r.second.bindings[1]);
					r.second.state = 6;
					//LOG(INFO) << "buttonInfo.state = 2: sendDigitalBinding, => 6";
				}
			}
		} break;
		case 3: {
			auto now = std::chrono::system_clock::now();
			if (r.second.timeout <= now) {
				sendDigitalBinding(r.second.remapping.binding, m_openvrId, ButtonEventType::ButtonPressed, (vr::EVRButtonId)r.first, 0.0, &r.second.bindings[0]);
				r.second.timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(100);
				r.second.state = 4;
				//LOG(INFO) << "buttonInfo.state = 3: sendDigitalBinding, => 4";
			}
		} break;
		case 4: {
			auto now = std::chrono::system_clock::now();
			if (r.second.timeout <= now) {
				sendDigitalBinding(r.second.remapping.binding, m_openvrId, ButtonEventType::ButtonUnpressed, (vr::EVRButtonId)r.first, 0.0, &r.second.bindings[0]);
				r.second.state = 0;
				//LOG(INFO) << "buttonInfo.state = 4: sendDigitalBinding, => 0";
			}
		} break;
		case 5: {
			if (r.second.remapping.doublePressImmediateRelease) {
				auto now = std::chrono::system_clock::now();
				if (r.second.timeout <= now) {
					sendDigitalBinding(r.second.remapping.doublePressBinding, m_openvrId, ButtonEventType::ButtonUnpressed, (vr::EVRButtonId)r.first, 0.0, &r.second.bindings[2]);
					r.second.state = 6;
					//LOG(INFO) << "buttonInfo.state = 5: sendDigitalBinding, => 6";
				}
			}
		} break;
		default:
			break;
		}
		RunFrameDigitalBinding(r.second.remapping.binding, (vr::EVRButtonId)r.first, r.second.bindings[0]);
		RunFrameDigitalBinding(r.second.remapping.longPressBinding, (vr::EVRButtonId)r.first, r.second.bindings[1]);
		RunFrameDigitalBinding(r.second.remapping.doublePressBinding, (vr::EVRButtonId)r.first, r.second.bindings[2]);
	}
}


void DeviceManipulationHandle::RunFrameDigitalBinding(vrinputemulator::DigitalBinding& binding, vr::EVRButtonId eButtonId, DeviceManipulationHandle::DigitalInputRemappingInfo::BindingInfo& bindingInfo) {
	if (bindingInfo.autoTriggerEnabled) {
		auto now = std::chrono::system_clock::now();
		if (bindingInfo.autoTriggerState && bindingInfo.autoTriggerUnpressTimeout < now) {
			bindingInfo.autoTriggerState = false;
			sendDigitalBinding(binding, m_openvrId, ButtonEventType::ButtonUnpressed, eButtonId, 0.0);
		} else if (!bindingInfo.autoTriggerState && bindingInfo.autoTriggerTimeout < now) {
			bindingInfo.autoTriggerState = true;
			auto now = std::chrono::system_clock::now();
			bindingInfo.autoTriggerUnpressTimeout = now + std::chrono::milliseconds(10);
			bindingInfo.autoTriggerTimeout = now + std::chrono::milliseconds(bindingInfo.autoTriggerTimeoutTime);
			sendDigitalBinding(binding, m_openvrId, ButtonEventType::ButtonPressed, eButtonId, 0.0);
		}
	}
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


bool DeviceManipulationHandle::handleAxisUpdate(uint32_t& unWhichDevice, uint32_t& unWhichAxis, vr::VRControllerAxis_t& axisState) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	AnalogInputRemappingInfo* axisInfo = nullptr;
	if (unWhichAxis < 5) {
		axisInfo = m_analogInputRemapping + unWhichAxis;
	}
	if (m_deviceMode == 1 || (m_deviceMode == 3 && !m_redirectSuspended) /*|| m_deviceMode == 5*/) {
		return false;
	}
	if (axisInfo && axisInfo->remapping.valid) {
		sendAnalogBinding(axisInfo->remapping.binding, unWhichDevice, unWhichAxis, axisState, &axisInfo->binding);
	} else {
		sendAxisEvent(unWhichDevice, unWhichAxis, axisState);
	}
	return false;
}


bool DeviceManipulationHandle::handleBooleanComponentUpdate(vr::VRInputComponentHandle_t& ulComponent, bool& bNewValue, double& fTimeOffset) {
	LOG(DEBUG) << "DeviceManipulationHandle::handleBooleanComponentUpdate(" << ulComponent << ", " << bNewValue << ", " << fTimeOffset << ")";
	auto it = _componentHandleToButtonIdMap.find(ulComponent);
	if (it != _componentHandleToButtonIdMap.end()) {
		ButtonEventType eventType;
		if (it->second.second == 0) { // touch
			eventType = bNewValue ? ButtonEventType::ButtonTouched : ButtonEventType::ButtonUntouched;
		} else { // press
			eventType = bNewValue ? ButtonEventType::ButtonPressed : ButtonEventType::ButtonUnpressed;
		}
		return handleButtonEvent(m_openvrId, eventType, it->second.first, fTimeOffset);
	} else {
		LOG(INFO) << "No mapping from boolean component handle " << ulComponent << " to button id";
	}
	return true;
}


bool DeviceManipulationHandle::handleScalarComponentUpdate(vr::VRInputComponentHandle_t& ulComponent, float& fNewValue, double& fTimeOffset) {
	LOG(DEBUG) << "DeviceManipulationHandle::handleScalarComponentUpdate(" << ulComponent << ", " << fNewValue << ", " << fTimeOffset << ")";
	auto it = _componentHandleToAxisIdMap.find(ulComponent);
	if (it != _componentHandleToAxisIdMap.end()) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		AnalogInputRemappingInfo* axisInfo = nullptr;
		unsigned unWhichAxis = it->second.first;
		unsigned unWhichAxisDim = it->second.second;
		if (unWhichAxis < 5) {
			axisInfo = m_analogInputRemapping + unWhichAxis;
		}
		if (m_deviceMode == 1 || (m_deviceMode == 3 && !m_redirectSuspended) /*|| m_deviceMode == 5*/) {
			return false;
		}
		if (axisInfo && axisInfo->remapping.valid) {
			sendAnalogBinding(axisInfo->remapping.binding, m_openvrId, unWhichAxis, unWhichAxisDim, ulComponent, fNewValue, fTimeOffset);
		} else {
			sendScalarComponentUpdate(m_openvrId, unWhichAxis, unWhichAxisDim, ulComponent, fNewValue, fTimeOffset);
		}
		return false;
	} else {
		LOG(INFO) << "No mapping from scalar component handle " << ulComponent << " to axis id";
	}
	return true;
}

bool DeviceManipulationHandle::handleHapticPulseEvent(float& fDurationSeconds, float& fFrequency, float& fAmplitude) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if ((m_deviceMode == 3 && !m_redirectSuspended) || m_deviceMode == 4) {
		m_redirectRef->ll_sendHapticPulseEvent(fDurationSeconds, fFrequency, fAmplitude);
		return false;
	} else  if (m_deviceMode == 0 || ((m_deviceMode == 3 || m_deviceMode == 2) && m_redirectSuspended)) {
		return true;
	}
	return false;
}


bool DeviceManipulationHandle::triggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds, bool directMode) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (directMode) {
		return ll_triggerHapticPulse(unAxisId, usPulseDurationMicroseconds);
	} else if ((m_deviceMode == 3 && !m_redirectSuspended) || m_deviceMode == 4) {
		return m_redirectRef->ll_triggerHapticPulse(unAxisId, usPulseDurationMicroseconds);
	} else  if (m_deviceMode == 0 || ((m_deviceMode == 3 || m_deviceMode == 2) && m_redirectSuspended)) {
		return ll_triggerHapticPulse(unAxisId, usPulseDurationMicroseconds);
	}
	return true;
}


bool _matchInputComponentName(const char* name, std::string& segment0, std::string& segment1, std::string& segment2, std::string& segment3) {
	boost::regex rgx("^/([^/]*)(/([^/]*))?(/([^/]*))?(/([^/]*))?");
	boost::smatch match;
	std::string text(name);
	if (boost::regex_search(text, match, rgx)) {
		segment0 = match[1];
		segment1 = match[3];
		segment2 = match[5];
		segment3 = match[7];
		return true;
	} else {
		return false;
	}
}

std::map<std::string, vr::EVRButtonId> _inputComponentNameToButtonId = {
	{ "system", vr::k_EButton_System },
	{ "application_menu", vr::k_EButton_ApplicationMenu },
	{ "grip", vr::k_EButton_Grip },
	{ "dpad_left/", vr::k_EButton_DPad_Left },
	{ "dpad_up", vr::k_EButton_DPad_Up },
	{ "dpad_right", vr::k_EButton_DPad_Right },
	{ "dpad_down", vr::k_EButton_DPad_Down },
	{ "a", vr::k_EButton_A },
	{ "x", vr::k_EButton_A },
	{ "b", vr::k_EButton_ApplicationMenu },
	{ "y", vr::k_EButton_ApplicationMenu },
	{ "trackpad", vr::k_EButton_SteamVR_Touchpad },
	{ "joystick", vr::k_EButton_SteamVR_Touchpad },
	{ "trigger", vr::k_EButton_SteamVR_Trigger },
};

std::map<std::string, uint32_t> _inputComponentNameToAxisId = {
	{ "trackpad", 0 },
	{ "joystick", 0 },
	{ "trigger", 1 },
};

void DeviceManipulationHandle::inputAddBooleanComponent(const char *pchName, uint64_t pHandle) {
	std::string sg0, sg1, sg2, sg3;
	if (_matchInputComponentName(pchName, sg0, sg1, sg2, sg3)) {
		LOG(DEBUG) << "Device Component Name Segments: \"" << sg0 << "\", \"" << sg1 << "\", \"" << sg2 << "\", \"" << sg3 << "\"";
		vr::EVRButtonId buttonId;
		int buttonType = 1; // 0 .. touch, 1 ..click
		bool errorFlag = false;
		if (!sg3.empty()) {
			LOG(ERROR) << "Device input component name \"" << pchName << "\" has too many segments.";
		} else {
			if (boost::iequals(sg0, "proximity")) { // proximity sensor
				buttonId = vr::k_EButton_ProximitySensor;
			} else if (boost::iequals(sg0, "input")) { // digital button
				boost::algorithm::to_lower(sg1);
				auto it = _inputComponentNameToButtonId.find(sg1);
				if (it != _inputComponentNameToButtonId.end()) {
					buttonId = it->second;
					if (boost::iequals(sg2, "touch")) {
						buttonType = 0;
					}
				} else {
					LOG(ERROR) << "Could not map input component name \"" << pchName << "\" to a button id.";
					errorFlag = true;
				}
			} else {
				LOG(ERROR) << "Unknown first component name segment \"" << sg0 << "\".";
				errorFlag = true;
			}
		}
		if (!errorFlag) {
			_componentHandleToButtonIdMap.emplace(pHandle, std::make_pair(buttonId, buttonType));
			if (buttonType == 0) {
				_ButtonIdToComponentHandleMap[buttonId].first = pHandle;
			} else {
				_ButtonIdToComponentHandleMap[buttonId].second = pHandle;
			}
			LOG(INFO) << "Mapped input component \"" << pchName << "\" to button id (" << (int)buttonId << ", " << buttonType << ")";
		}
	} else {
		LOG(ERROR) << "Could not parse input component name \"" << pchName << "\".";
	}
}

void DeviceManipulationHandle::inputAddScalarComponent(const char *pchName, uint64_t pHandle, vr::EVRScalarType eType, vr::EVRScalarUnits eUnits) {
	std::string sg0, sg1, sg2, sg3;
	if (_matchInputComponentName(pchName, sg0, sg1, sg2, sg3)) {
		LOG(DEBUG) << "Device Component Name Segments: \"" << sg0 << "\", \"" << sg1 << "\", \"" << sg2 << "\", \"" << sg3 << "\"";
		uint32_t axisId;
		uint32_t axisDim = 0;
		bool errorFlag = false;
		if (!sg3.empty()) {
			LOG(ERROR) << "Device input component name \"" << pchName << "\" has too many segments.";
		} else {
			if (boost::iequals(sg0, "input")) { // analog input
				boost::algorithm::to_lower(sg1);
				auto it = _inputComponentNameToAxisId.find(sg1);
				if (it != _inputComponentNameToAxisId.end()) {
					axisId = it->second;
					if (boost::iequals(sg2, "x")) {
						axisDim = 0;
					} else if (boost::iequals(sg2, "y")) {
						axisDim = 1;
					}
				} else {
					LOG(ERROR) << "Could not map input component name \"" << pchName << "\" to an axis id.";
					errorFlag = true;
				}
			} else {
				LOG(ERROR) << "Unknown first component name segment \"" << sg0 << "\".";
				errorFlag = true;
			}
		}
		if (!errorFlag) {
			_componentHandleToAxisIdMap.emplace(pHandle, std::make_pair(axisId, axisDim));
			if (axisDim == 0) {
				_AxisIdToComponentHandleMap[axisId].first = pHandle;
			} else {
				_AxisIdToComponentHandleMap[axisId].second = pHandle;
			}
			LOG(INFO) << "Mapped input component \"" << pchName << "\" to axis id (" << axisId << ", " << axisDim << ")";
		}
	} else {
		LOG(ERROR) << "Could not parse input component name \"" << pchName << "\".";
	}
}

void DeviceManipulationHandle::inputAddHapticComponent(const char * pchName, uint64_t pHandle) {
	m_inputHapticComponentHandle = pHandle;
}



void DeviceManipulationHandle::sendDigitalBinding(vrinputemulator::DigitalBinding& binding, uint32_t unWhichDevice, ButtonEventType eventType, 
		vr::EVRButtonId eButtonId, double eventTimeOffset, DigitalInputRemappingInfo::BindingInfo* bindingInfo) {
	if (binding.type == DigitalBindingType::NoRemapping) {
		sendButtonEvent(unWhichDevice, eventType, eButtonId, eventTimeOffset, false, bindingInfo);
	} else if (binding.type == DigitalBindingType::Disabled) {
		// nop
	} else {
		bool sendEvent = false;
		if (!bindingInfo) {
			sendEvent = true;
		} else if (eventType == ButtonEventType::ButtonPressed || eventType == ButtonEventType::ButtonUnpressed) {
			switch (bindingInfo->state) {
				case 0:  {
					int newState = 1;
					if (eventType == ButtonEventType::ButtonPressed) {
						sendEvent = true;
						if (binding.toggleEnabled) {
							if (binding.toggleDelay == 0) {
								newState = 2;
							} else {
								bindingInfo->timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(binding.toggleDelay);
							}
						}
						bindingInfo->autoTriggerEnabled = binding.autoTriggerEnabled;
						if (bindingInfo->autoTriggerEnabled) {
							bindingInfo->autoTriggerState = true;
							bindingInfo->autoTriggerTimeoutTime = (uint32_t)(1000.0 / ((float)binding.autoTriggerFrequency / 100.0));
							auto now = std::chrono::system_clock::now();
							bindingInfo->autoTriggerUnpressTimeout = now + std::chrono::milliseconds(10);
							bindingInfo->autoTriggerTimeout = now + std::chrono::milliseconds(bindingInfo->autoTriggerTimeoutTime);
						}
						bindingInfo->state = newState;
					}
				} break;
				case 1: {
					if (eventType == ButtonEventType::ButtonUnpressed) {
						sendEvent = true;
						if (bindingInfo->autoTriggerEnabled) {
							bindingInfo->autoTriggerEnabled = false;
							if (!bindingInfo->autoTriggerState) {
								bindingInfo->pressedState = false;
							}
						}
						bindingInfo->state = 0;
					}
				} break;
				case 2: {
					if (eventType == ButtonEventType::ButtonPressed) {
						bindingInfo->state = 3;
					}
				} break;
				case 3: {
					if (eventType == ButtonEventType::ButtonUnpressed) {
						sendEvent = true;
						if (bindingInfo->autoTriggerEnabled) {
							bindingInfo->autoTriggerEnabled = false;
							bindingInfo->autoTriggerState = false;
						}
						bindingInfo->state = 0;
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
						vr::EVRButtonId button = (vr::EVRButtonId)binding.data.openvr.buttonId;
						uint32_t deviceId = binding.data.openvr.controllerId;
						if (deviceId >= 999) {
							deviceId = m_openvrId;
						}
						sendButtonEvent(deviceId, eventType, button, eventTimeOffset, false, bindingInfo);
					}
				} break;
				case DigitalBindingType::Keyboard: {
					if (m_deviceMode == 1 || (m_deviceMode == 3 && !m_redirectSuspended) /*|| m_deviceMode == 5*/) {
						//nop
					} else {
						sendKeyboardEvent(eventType, binding.data.keyboard.shiftPressed, binding.data.keyboard.ctrlPressed, 
							binding.data.keyboard.altPressed, (WORD)binding.data.keyboard.keyCode, binding.data.keyboard.sendScanCode, bindingInfo);
					}
				} break;
				case DigitalBindingType::SuspendRedirectMode: {
					if (eventType == ButtonEventType::ButtonPressed) {
						//_vibrationCue(); // Better not let it interfere with an haptic event triggered by an application
						_audioCue();
						suspendRedirectMode();
						bindingInfo->pressedState = true;
					} else if (eventType == ButtonEventType::ButtonUnpressed) {
						bindingInfo->pressedState = false;
					}
				} break;
				case DigitalBindingType::ToggleTouchpadEmulationFix: {
					if (eventType == ButtonEventType::ButtonPressed) {
						//_vibrationCue(); // Better not let it interfere with an haptic event triggered by an application
						_audioCue();
						DeviceManipulationHandle::setTouchpadEmulationFixFlag(!DeviceManipulationHandle::getTouchpadEmulationFixFlag());
						bindingInfo->pressedState = true;
					} else if (eventType == ButtonEventType::ButtonUnpressed) {
						bindingInfo->pressedState = false;
					}
				} break;
				default: {
				} break;
			}
		}
	}
}


void DeviceManipulationHandle::_audioCue() {
	auto audiofile = ServerDriver::getInstallDirectory().append("\\resources\\sounds\\audiocue.wav");
	PlaySoundA(audiofile.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}


void DeviceManipulationHandle::_vibrationCue() {
	if (_vibrationCueTheadHandle != NULL) {
		DWORD threadExitCode = 0;
		if (GetExitCodeThread(_vibrationCueTheadHandle, &threadExitCode) != 0 && threadExitCode == STILL_ACTIVE) {
			return;
		}
		CloseHandle(_vibrationCueTheadHandle);
		_vibrationCueTheadHandle = NULL;
	}
	static struct _vibrationCueThreadArgs {
		DeviceManipulationHandle* deviceInfo;
	} threadArgs;
	threadArgs.deviceInfo = this;
	_vibrationCueTheadHandle = CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		[](LPVOID lpParam)->DWORD {
			_vibrationCueThreadArgs* args = (_vibrationCueThreadArgs*)lpParam;
			for (unsigned i = 0; i < 10; ++i) {
				args->deviceInfo->ll_triggerHapticPulse(0, 1000);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			return 0;
		},
		&threadArgs,			// argument to thread function 
		0,                      // use default creation flags 
		NULL
	);
}


void DeviceManipulationHandle::sendAnalogBinding(vrinputemulator::AnalogBinding& binding, uint32_t unWhichDevice, uint32_t axisId, const vr::VRControllerAxis_t& axisState, DeviceManipulationHandle::AnalogInputRemappingInfo::BindingInfo* bindingInfo) {
	if (binding.type == AnalogBindingType::NoRemapping) {
		sendAxisEvent(unWhichDevice, axisId, axisState, false, bindingInfo);
	} else if (binding.type == AnalogBindingType::Disabled) {
		// nop
	} else {
		switch (binding.type) {
			case AnalogBindingType::OpenVR: {
				if (m_deviceMode == 1 || (m_deviceMode == 3 && !m_redirectSuspended) /*|| m_deviceMode == 5*/) {
					//nop
				} else {
					vr::EVRButtonId axisId = (vr::EVRButtonId)binding.data.openvr.axisId;
					uint32_t deviceId = binding.data.openvr.controllerId;
					auto newAxisState = axisState;
					if (deviceId >= 999) {
						deviceId = m_openvrId;
					}
					if (binding.invertXAxis) {
						newAxisState.x *= -1;
					}
					if (binding.invertYAxis) {
						newAxisState.y *= -1;
					}
					if (binding.swapAxes) {
						auto tmp = newAxisState.x;
						newAxisState.x = newAxisState.y;
						newAxisState.y = tmp;
					}
					if ( binding.lowerDeadzone < binding.upperDeadzone && (binding.lowerDeadzone > 0.0f || binding.upperDeadzone < 1.0f)) {
						auto zone = binding.upperDeadzone - binding.lowerDeadzone;
						if (axisState.x >= 0.0f) {
							newAxisState.x = (axisState.x - binding.lowerDeadzone) * (1.0f / zone) + binding.lowerDeadzone;
							if (newAxisState.x < 0.0f) {
								newAxisState.x = 0.0f;
							} else if (newAxisState.x > 1.0f) {
								newAxisState.x = 1.0f;
							}
						} else {
							newAxisState.x = (axisState.x + binding.lowerDeadzone) * (1.0f / zone) - binding.lowerDeadzone;
							if (newAxisState.x > 0.0f) {
								newAxisState.x = 0.0f;
							} else if (newAxisState.x < -1.0f) {
								newAxisState.x = -1.0f;
							}
						}
						if (axisState.y >= 0.0f) {
							newAxisState.y = (axisState.y - binding.lowerDeadzone) * (1.0f / zone) + binding.lowerDeadzone;
							if (newAxisState.y < 0.0f) {
								newAxisState.y = 0.0f;
							} else if (newAxisState.y > 1.0f) {
								newAxisState.y = 1.0f;
							}
						} else {
							newAxisState.y = (axisState.y + binding.lowerDeadzone) * (1.0f / zone) - binding.lowerDeadzone;
							if (newAxisState.y > 0.0f) {
								newAxisState.y = 0.0f;
							} else if (newAxisState.y < -1.0f) {
								newAxisState.y = -1.0f;
							}
						}
					}
					sendAxisEvent(deviceId, axisId, newAxisState, false, bindingInfo);
				}
			} break;
			default: {
			} break;
		}
	}
}

void DeviceManipulationHandle::sendAnalogBinding(vrinputemulator::AnalogBinding & binding, uint32_t unWhichDevice, uint32_t unWhichAxis, uint32_t unAxisDim, vr::VRInputComponentHandle_t ulComponent, float fNewValue, double fTimeOffset) {
	if (binding.type == AnalogBindingType::NoRemapping) {
		sendScalarComponentUpdate(unWhichDevice, unWhichAxis, unAxisDim, ulComponent, fNewValue, fTimeOffset);
	} else if (binding.type == AnalogBindingType::Disabled) {
		// nop
	} else {
		switch (binding.type) {
			case AnalogBindingType::OpenVR: {
				if (m_deviceMode == 1 || (m_deviceMode == 3 && !m_redirectSuspended) /*|| m_deviceMode == 5*/) {
					//nop
				} else {
					uint32_t axisId = binding.data.openvr.axisId;
					uint32_t axisDim = unAxisDim;
					uint32_t deviceId = binding.data.openvr.controllerId;
					auto myNewState = fNewValue;
					if (deviceId >= 999) {
						deviceId = m_openvrId;
					}
					if ((unAxisDim == 0 && binding.invertXAxis) || (unAxisDim == 1 && binding.invertYAxis)) {
						myNewState *= -1;
					}
					if (binding.swapAxes) {
						if (axisDim == 0) {
							axisDim = 1;
						} else {
							axisDim = 0;
						}
					}
					if (binding.lowerDeadzone < binding.upperDeadzone && (binding.lowerDeadzone > 0.0f || binding.upperDeadzone < 1.0f)) {
						auto zone = binding.upperDeadzone - binding.lowerDeadzone;
						if (myNewState >= 0.0f) {
							myNewState = (myNewState - binding.lowerDeadzone) * (1.0f / zone) + binding.lowerDeadzone;
							if (myNewState < 0.0f) {
								myNewState = 0.0f;
							} else if (myNewState > 1.0f) {
								myNewState = 1.0f;
							}
						} else {
							myNewState = (myNewState + binding.lowerDeadzone) * (1.0f / zone) - binding.lowerDeadzone;
							if (myNewState > 0.0f) {
								myNewState = 0.0f;
							} else if (myNewState < -1.0f) {
								myNewState = -1.0f;
							}
						}
					}
					if (deviceId != unWhichDevice || axisId != unWhichAxis || axisDim != unAxisDim) {
						sendScalarComponentUpdate(deviceId, axisId, unAxisDim, myNewState, fTimeOffset);
					} else {
						sendScalarComponentUpdate(deviceId, axisId, unAxisDim, ulComponent, myNewState, fTimeOffset);
					}
				}
			} break;
			default: {
			} break;
		}
	}
}


void DeviceManipulationHandle::_buttonPressDeadzoneFix(vr::EVRButtonId eButtonId) {
	auto& axisInfo = m_analogInputRemapping[eButtonId - vr::k_EButton_Axis0];
	if (axisInfo.remapping.valid && axisInfo.remapping.binding.buttonPressDeadzoneFix) {
		if (axisInfo.binding.lastSendAxisState.x == 0.0f && axisInfo.binding.lastSendAxisState.y == 0.0f) {
			ll_sendAxisEvent(eButtonId - vr::k_EButton_Axis0, { 0.01f, 0.01f });
		}
	}
}

void DeviceManipulationHandle::sendButtonEvent(uint32_t unWhichDevice, ButtonEventType eventType, vr::EVRButtonId eButtonId, double eventTimeOffset, bool directMode, DigitalInputRemappingInfo::BindingInfo* binding) {
	if (!directMode) {
		if (unWhichDevice == m_openvrId && ((m_deviceMode == 2 && !m_redirectSuspended) || m_deviceMode == 4)) {
			unWhichDevice = m_redirectRef->openvrId();
		}
		if (unWhichDevice != m_openvrId) {
			auto deviceInfo = m_parent->getDeviceManipulationHandleById(unWhichDevice);
			if (deviceInfo) {
				return deviceInfo->sendButtonEvent(unWhichDevice, eventType, eButtonId, eventTimeOffset, true, binding);
			}
		}
	}
	switch (eventType) {
		case ButtonEventType::ButtonPressed: {
			if (binding) {
				if (!binding->touchedState) {
					if (eButtonId >= vr::k_EButton_Axis0 && eButtonId <= vr::k_EButton_Axis4) {
						auto& axisInfo = m_analogInputRemapping[eButtonId - vr::k_EButton_Axis0];
						axisInfo.binding.touchedState = true;
					}
					ll_sendButtonEvent(ButtonEventType::ButtonTouched, eButtonId, eventTimeOffset);
					binding->touchedState = true;
					binding->touchedAutoset = true;
				}
				if (!binding->pressedState) {
					if (eButtonId >= vr::k_EButton_Axis0 && eButtonId <= vr::k_EButton_Axis4) {
						_buttonPressDeadzoneFix(eButtonId);
					}
					ll_sendButtonEvent(ButtonEventType::ButtonPressed, eButtonId, eventTimeOffset);
					binding->pressedState = true;
				}
			} else {
				if (eButtonId >= vr::k_EButton_Axis0 && eButtonId <= vr::k_EButton_Axis4) {
					_buttonPressDeadzoneFix(eButtonId);
				}
				ll_sendButtonEvent(ButtonEventType::ButtonPressed, eButtonId, eventTimeOffset);
			}
		} break;
		case ButtonEventType::ButtonUnpressed: {
			if (binding) {
				if (binding->pressedState) {
					ll_sendButtonEvent(ButtonEventType::ButtonUnpressed, eButtonId, eventTimeOffset);
					binding->pressedState = false;
				}
				if (binding->touchedState && binding->touchedAutoset) {
					ll_sendButtonEvent(ButtonEventType::ButtonUntouched, eButtonId, eventTimeOffset);
					binding->touchedState = false;
					binding->touchedAutoset = false;
				}
			} else {
				ll_sendButtonEvent(ButtonEventType::ButtonUnpressed, eButtonId, eventTimeOffset);
			}
		} break;
		case ButtonEventType::ButtonTouched: {
			if (binding) {
				if (!binding->touchedState) {
					ll_sendButtonEvent(ButtonEventType::ButtonTouched, eButtonId, eventTimeOffset);
					binding->touchedState = true;
					binding->touchedAutoset = false;
				}
			} else {
				ll_sendButtonEvent(ButtonEventType::ButtonTouched, eButtonId, eventTimeOffset);
			}
		} break;
		case ButtonEventType::ButtonUntouched: {
			if (binding) {
				if (binding->touchedState) {
					if (binding->pressedState) {
						binding->touchedAutoset = true;
					} else {
						ll_sendButtonEvent(ButtonEventType::ButtonUntouched, eButtonId, eventTimeOffset);
						binding->touchedState = false;
						binding->touchedAutoset = false;
					}
				}
			} else {
				ll_sendButtonEvent(ButtonEventType::ButtonUntouched, eButtonId, eventTimeOffset);
			}
		} break;
		default: {
		} break;
	}
}


void DeviceManipulationHandle::sendKeyboardEvent(ButtonEventType eventType, bool shiftPressed, bool ctrlPressed, bool altPressed, WORD keyCode, bool sendScanCode, DigitalInputRemappingInfo::BindingInfo* binding) {
	if ( (eventType == ButtonEventType::ButtonPressed && (!binding || !binding->pressedState)) 
			|| (eventType == ButtonEventType::ButtonUnpressed && (!binding || binding->pressedState)) ) {
		INPUT ips[4];
		memset(ips, 0, sizeof(ips));
		WORD flags = 0;
		/* DirectInput games ignore virtual key codes, for those scan codes should be used */
		if (sendScanCode) {
			flags |= KEYEVENTF_SCANCODE;
			keyCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
		}
		if (eventType == ButtonEventType::ButtonUnpressed) {
			flags |= KEYEVENTF_KEYUP;
		}
		unsigned ipCount = 0;
		if (eventType == ButtonEventType::ButtonUnpressed && keyCode) {
			ips[ipCount].type = INPUT_KEYBOARD;
			if (sendScanCode) {
				ips[ipCount].ki.wScan = keyCode;
			} else {
				ips[ipCount].ki.wVk = keyCode;
			}
			ips[ipCount].ki.dwFlags = flags;
			ipCount++;
		}
		if (shiftPressed) {
			ips[ipCount].type = INPUT_KEYBOARD;
			if (sendScanCode) {
				ips[ipCount].ki.wScan = MapVirtualKey(VK_SHIFT, MAPVK_VK_TO_VSC);
			} else {
				ips[ipCount].ki.wVk = VK_SHIFT;
			}
			ips[ipCount].ki.dwFlags = flags;
			ipCount++;
		}
		if (ctrlPressed) {
			ips[ipCount].type = INPUT_KEYBOARD;
			if (sendScanCode) {
				ips[ipCount].ki.wScan = MapVirtualKey(VK_CONTROL, MAPVK_VK_TO_VSC);
			} else {
				ips[ipCount].ki.wVk = VK_CONTROL;
			}
			ips[ipCount].ki.dwFlags = flags;
			ipCount++;
		}
		if (altPressed) {
			ips[ipCount].type = INPUT_KEYBOARD;
			if (sendScanCode) {
				ips[ipCount].ki.wScan = MapVirtualKey(VK_MENU, MAPVK_VK_TO_VSC);
			} else {
				ips[ipCount].ki.wVk = VK_MENU;
			}
			ips[ipCount].ki.dwFlags = flags;
			ipCount++;
		}
		if (eventType == ButtonEventType::ButtonPressed && keyCode) {
			ips[ipCount].type = INPUT_KEYBOARD;
			if (sendScanCode) {
				ips[ipCount].ki.wScan = keyCode;
			} else {
				ips[ipCount].ki.wVk = keyCode;
			}
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


void DeviceManipulationHandle::sendAxisEvent(uint32_t unWhichDevice, uint32_t unWhichAxis, const vr::VRControllerAxis_t& axisState, bool directMode, AnalogInputRemappingInfo::BindingInfo* binding) {
	if (!directMode) {
		if (unWhichDevice == m_openvrId && ((m_deviceMode == 2 && !m_redirectSuspended) || m_deviceMode == 4)) {
			unWhichDevice = m_redirectRef->openvrId();
		}
		if (unWhichDevice != m_openvrId) {
			auto deviceInfo = m_parent->getDeviceManipulationHandleById(unWhichDevice);
			if (deviceInfo) {
				return deviceInfo->sendAxisEvent(unWhichDevice, unWhichAxis, axisState, true, binding);
			}
		}
	}
	if (unWhichAxis < 5) {
		if (m_analogInputRemapping[unWhichAxis].remapping.valid && touchpadEmulationEnabledFlag) {
			auto newState = axisState;
			auto& lastSeenState = m_analogInputRemapping[unWhichAxis].binding.lastSeenAxisState;
			auto& lastSendState = m_analogInputRemapping[unWhichAxis].binding.lastSendAxisState;
			bool suppress = false;

			if (m_analogInputRemapping[unWhichAxis].remapping.binding.touchpadEmulationMode == 1) {
				if (axisState.x != 0.0f && vrmath::signum(axisState.x) == vrmath::signum(lastSendState.x) && abs(axisState.x) < abs(lastSendState.x)) {
					newState.x = lastSendState.x;
				}
				if (axisState.y != 0.0f && vrmath::signum(axisState.y) == vrmath::signum(lastSendState.y) && abs(axisState.y) < abs(lastSendState.y)) {
					newState.y = lastSendState.y;
				}

			} else if (m_analogInputRemapping[unWhichAxis].remapping.binding.touchpadEmulationMode == 2) {
				// Joystick was already in neutral position but we haven't send it yet, and now it moved away from neutral position
				// => send neutral position before we do anything else since some menus use this information to reset input handling.
				if (lastSeenState.x == 0.0f && lastSeenState.y == 0.0f && (lastSendState.x != 0.0f || lastSendState.y != 0.0f) && (axisState.x != 0.0f || axisState.y != 0.0f)) {
					ll_sendAxisEvent(unWhichAxis, { 0.0f, 0.0f });
					lastSendState = {0.0f, 0.0f};
				}
				if (axisState.x == 0.0f || (vrmath::signum(axisState.x) == vrmath::signum(lastSendState.x) && abs(axisState.x) < abs(lastSendState.x))) {
					newState.x = lastSendState.x;
				}
				if (axisState.y == 0.0f || (vrmath::signum(axisState.y) == vrmath::signum(lastSendState.y) && abs(axisState.y) < abs(lastSendState.y))) {
					newState.y = lastSendState.y;
				}
			}

			lastSeenState = axisState;
			if (!suppress) {
				ll_sendAxisEvent(unWhichAxis, newState);
				lastSendState = newState;
			}
		} else {
			ll_sendAxisEvent(unWhichAxis, axisState);
			m_analogInputRemapping[unWhichAxis].binding.lastSeenAxisState = axisState;
			m_analogInputRemapping[unWhichAxis].binding.lastSendAxisState = axisState;
		}

	} else {
		ll_sendAxisEvent(unWhichAxis, axisState);
	}
}

void DeviceManipulationHandle::sendScalarComponentUpdate(uint32_t unWhichDevice, uint32_t unWhichAxis, uint32_t unAxisDim, vr::VRInputComponentHandle_t ulComponent, float fNewValue, double fTimeOffset, bool directMode) {
	if (!directMode) {
		if (unWhichDevice == m_openvrId && ((m_deviceMode == 2 && !m_redirectSuspended) || m_deviceMode == 4)) {
			unWhichDevice = m_redirectRef->openvrId();
		}
		if (unWhichDevice != m_openvrId) {
			auto deviceInfo = m_parent->getDeviceManipulationHandleById(unWhichDevice);
			if (deviceInfo) {
				return deviceInfo->sendScalarComponentUpdate(unWhichDevice, unWhichAxis, unAxisDim, fNewValue, fTimeOffset, true);
			}
		}
	}
	if (unWhichAxis < 5) {
		auto& axisInfo = m_analogInputRemapping[unWhichAxis];
		if (axisInfo.remapping.valid && touchpadEmulationEnabledFlag) {
			auto myNewState = fNewValue;
			auto& lastSeenState = m_analogInputRemapping[unWhichAxis].binding.lastSeenAxisState;
			auto& lastSendState = m_analogInputRemapping[unWhichAxis].binding.lastSendAxisState;
			bool suppress = false;

			if (m_analogInputRemapping[unWhichAxis].remapping.binding.touchpadEmulationMode == 1) {
				if (unAxisDim == 0 && (fNewValue != 0.0f && vrmath::signum(fNewValue) == vrmath::signum(lastSendState.x) && abs(fNewValue) < abs(lastSendState.x))) {
					myNewState = lastSendState.x;
				} else if (unAxisDim == 1 && (fNewValue != 0.0f && vrmath::signum(fNewValue) == vrmath::signum(lastSendState.y) && abs(fNewValue) < abs(lastSendState.y))) {
					myNewState = lastSendState.y;
				}

			} else if (m_analogInputRemapping[unWhichAxis].remapping.binding.touchpadEmulationMode == 2) {
				// Joystick was already in neutral position but we haven't send it yet, and now it moved away from neutral position
				// => send neutral position before we do anything else since some menus use this information to reset input handling.
				if (lastSeenState.x == 0.0f && lastSeenState.y == 0.0f && (lastSendState.x != 0.0f || lastSendState.y != 0.0f) && fNewValue != 0.0f) {
					ll_sendScalarComponentUpdate(ulComponent, 0.0f, fTimeOffset);
					if (unAxisDim == 0) {
						ll_sendScalarComponentUpdate(_AxisIdToComponentHandleMap[unWhichAxis].second, 0.0f, fTimeOffset);
					} else {
						ll_sendScalarComponentUpdate(_AxisIdToComponentHandleMap[unWhichAxis].first, 0.0f, fTimeOffset);
					}
					lastSendState = {0.0f, 0.0f};
				}
				if (unAxisDim == 0 && (fNewValue == 0.0f || (vrmath::signum(fNewValue) == vrmath::signum(lastSendState.x) && abs(fNewValue) < abs(lastSendState.x)))) {
					myNewState = lastSendState.x;
				} else if (unAxisDim == 1 && (fNewValue == 0.0f || (vrmath::signum(fNewValue) == vrmath::signum(lastSendState.y) && abs(fNewValue) < abs(lastSendState.y)))) {
					myNewState = lastSendState.y;
				}
			}

			if (unAxisDim == 0) {
				lastSeenState.x = fNewValue;
			} else {
				lastSeenState.y = fNewValue;
			}
			if (!suppress) {
				ll_sendScalarComponentUpdate(ulComponent, myNewState, fTimeOffset);
				if (unAxisDim == 0) {
					lastSendState.x = myNewState;
				} else {
					lastSendState.y = myNewState;
				}
			}
		} else {
			ll_sendScalarComponentUpdate(ulComponent, fNewValue, fTimeOffset);
			if (unAxisDim == 0) {
				axisInfo.binding.lastSeenAxisState.x = fNewValue;
				axisInfo.binding.lastSendAxisState.x = fNewValue;
			} else {
				axisInfo.binding.lastSeenAxisState.y = fNewValue;
				axisInfo.binding.lastSendAxisState.y = fNewValue;
			}
		}
	} else {
		ll_sendScalarComponentUpdate(ulComponent, fNewValue, fTimeOffset);
	}
}

void DeviceManipulationHandle::sendScalarComponentUpdate(uint32_t unWhichDevice, uint32_t unWhichAxis, uint32_t unAxisDim, float fNewValue, double fTimeOffset, bool directMode) {
	if (!directMode) {
		if (unWhichDevice == m_openvrId && ((m_deviceMode == 2 && !m_redirectSuspended) || m_deviceMode == 4)) {
			unWhichDevice = m_redirectRef->openvrId();
		}
		if (unWhichDevice != m_openvrId) {
			auto deviceInfo = m_parent->getDeviceManipulationHandleById(unWhichDevice);
			if (deviceInfo) {
				return deviceInfo->sendScalarComponentUpdate(unWhichDevice, unWhichAxis, unAxisDim, fNewValue, fTimeOffset, true);
			}
		}
	}
	if (unWhichAxis < 5) {
		uint64_t componentHandle = 0;
		if (unAxisDim == 0) {
			componentHandle = _AxisIdToComponentHandleMap[unWhichAxis].first;
		} else {
			componentHandle = _AxisIdToComponentHandleMap[unWhichAxis].second;
		}
		if (componentHandle != 0) {
			sendScalarComponentUpdate(unWhichDevice, unWhichAxis, unAxisDim, componentHandle, fNewValue, fTimeOffset, directMode);
		}
	}
}



int DeviceManipulationHandle::setDefaultMode() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	auto res = _disableOldMode(0);
	if (res == 0) {
		m_deviceMode = 0;
	}
	return 0; 
}

int DeviceManipulationHandle::setRedirectMode(bool target, DeviceManipulationHandle* ref) {
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

int DeviceManipulationHandle::setSwapMode(DeviceManipulationHandle* ref) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	auto res = _disableOldMode(4);
	if (res == 0) {
		m_redirectRef = ref;
		m_deviceMode = 4;
	}
	return 0;
}

int DeviceManipulationHandle::setMotionCompensationMode() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	auto res = _disableOldMode(5);
	auto serverDriver = ServerDriver::getInstance();
	if (res == 0 && serverDriver) {
		m_motionCompensationManager.enableMotionCompensation(true);
		m_motionCompensationManager.setMotionCompensationRefDevice(this);
		m_motionCompensationManager._setMotionCompensationStatus(MotionCompensationStatus::WaitingForZeroRef);
		m_deviceMode = 5;
	}
	return 0;
}

int DeviceManipulationHandle::setFakeDisconnectedMode() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	auto res = _disableOldMode(1);
	if (res == 0) {
		_disconnectedMsgSend = false;
		m_deviceMode = 1;
	}
	return 0;
}

int DeviceManipulationHandle::_disableOldMode(int newMode) {
	if (m_deviceMode != newMode) {
		if (m_deviceMode == 5) {
			auto serverDriver = ServerDriver::getInstance();
			if (serverDriver) {
				m_motionCompensationManager.enableMotionCompensation(false);
				m_motionCompensationManager.setMotionCompensationRefDevice(nullptr);
			}
		} else if (m_deviceMode == 3 || m_deviceMode == 2 || m_deviceMode == 4) {
			m_redirectRef->m_deviceMode = 0;
		}
		if (newMode == 5) {
			auto serverDriver = ServerDriver::getInstance();
			if (serverDriver) {
				m_motionCompensationManager._disableMotionCompensationOnAllDevices();
				m_motionCompensationManager.enableMotionCompensation(false);
				m_motionCompensationManager.setMotionCompensationRefDevice(nullptr);
			}
		} 
	}
	return 0;
}



} // end namespace driver
} // end namespace vrinputemulator
