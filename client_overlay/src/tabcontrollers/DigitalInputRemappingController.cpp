#include "DigitalInputRemappingController.h"
#include <QQuickWindow>
#include <QApplication>
#include "../overlaycontroller.h"
#include <openvr_math.h>
#include <vrinputemulator_types.h>

// application namespace
namespace inputemulator {

DigitalInputRemappingController::~DigitalInputRemappingController() {
}

void DigitalInputRemappingController::initStage1() {
}


void DigitalInputRemappingController::initStage2(OverlayController * parent, QQuickWindow * widget) {
	this->parent = parent;
	this->widget = widget;
}


void DigitalInputRemappingController::eventLoopTick(vr::TrackedDevicePose_t* /*devicePoses*/) {
	if (settingsUpdateCounter >= 50) {
		settingsUpdateCounter = 0;
	} else {
		settingsUpdateCounter++;
	}
}

void DigitalInputRemappingController::handleEvent(const vr::VREvent_t&) {
	/*switch (vrEvent.eventType) {
	default:
	break;
	}*/
}

void DigitalInputRemappingController::startConfigureRemapping(vrinputemulator::DigitalInputRemapping remapping, uint32_t deviceIndex, uint32_t deviceId, uint32_t buttonId) {
	m_currentDeviceIndex = deviceIndex;
	m_currentDeviceId = deviceId;
	m_currentButtonId = buttonId;
	m_currentRemapping = remapping;
}

void DigitalInputRemappingController::startConfigureNormalBinding() {
	m_currentBinding = &m_currentRemapping.binding;
}

void DigitalInputRemappingController::startConfigureLongPressBinding() {
	m_currentBinding = &m_currentRemapping.longPressBinding;
}

void DigitalInputRemappingController::startConfigureDoublePressBinding() {
	m_currentBinding = &m_currentRemapping.doublePressBinding;
}

QString DigitalInputRemappingController::getNormalBindingStatus() {
	return parent->digitalBindingToString(m_currentRemapping.binding, m_currentRemapping.binding.data.openvr.controllerId != m_currentDeviceId);
}

bool DigitalInputRemappingController::isLongPressEnabled() {
	return m_currentRemapping.longPressEnabled;
}

unsigned DigitalInputRemappingController::getLongPressThreshold() {
	return m_currentRemapping.longPressThreshold;
}

QString DigitalInputRemappingController::getLongPressBindingStatus() {
	return parent->digitalBindingToString(m_currentRemapping.longPressBinding, m_currentRemapping.longPressBinding.data.openvr.controllerId != m_currentDeviceId);
}

bool DigitalInputRemappingController::isLongPressImmediateRelease() {
	return m_currentRemapping.longPressImmediateRelease;
}

bool DigitalInputRemappingController::isDoublePressEnabled() {
	return m_currentRemapping.doublePressEnabled;
}

unsigned DigitalInputRemappingController::getDoublePressThreshold() {
	return m_currentRemapping.doublePressThreshold;
}

QString DigitalInputRemappingController::getDoublePressBindingStatus() {
	return parent->digitalBindingToString(m_currentRemapping.doublePressBinding, m_currentRemapping.doublePressBinding.data.openvr.controllerId != m_currentDeviceId);
}

bool DigitalInputRemappingController::isDoublePressImmediateRelease() {
	return m_currentRemapping.doublePressImmediateRelease;
}

int DigitalInputRemappingController::getBindingType() {
	if (m_currentBinding != nullptr) {
		return (int)m_currentBinding->type;
	} else {
		return 0;
	}
}

int DigitalInputRemappingController::getBindingOpenVRControllerId() {
	if (m_currentBinding) {
		return m_currentBinding->data.openvr.controllerId;
	} else {
		return -1;
	}
}

int DigitalInputRemappingController::getBindingOpenVRButtonId() {
	if (m_currentBinding) {
		return m_currentBinding->data.openvr.buttonId;
	} else {
		return -1;
	}
}

bool DigitalInputRemappingController::touchAsClickEnabled() {
	if (m_currentRemapping.valid) {
		return m_currentRemapping.touchAsClick;
	} else {
		return false;
	}
}

bool DigitalInputRemappingController::isToggleModeEnabled() {
	if (m_currentBinding) {
		return m_currentBinding->toggleEnabled;
	} else {
		return false;
	}
}

int DigitalInputRemappingController::toggleModeThreshold() {
	if (m_currentBinding) {
		return m_currentBinding->toggleDelay;
	} else {
		return 0;
	}
}

bool DigitalInputRemappingController::isAutoTriggerEnabled() {
	if (m_currentBinding) {
		return m_currentBinding->autoTriggerEnabled;
	} else {
		return false;
	}
}

int DigitalInputRemappingController::autoTriggerFrequency() {
	if (m_currentBinding) {
		return m_currentBinding->autoTriggerFrequency;
	} else {
		return 0;
	}
}

bool DigitalInputRemappingController::keyboardShiftEnabled() {
	if (m_currentBinding) {
		return m_currentBinding->data.keyboard.shiftPressed;
	} else {
		return false;
	}
}

bool DigitalInputRemappingController::keyboardCtrlEnabled() {
	if (m_currentBinding) {
		return m_currentBinding->data.keyboard.ctrlPressed;
	} else {
		return false;
	}
}

bool DigitalInputRemappingController::keyboardAltEnabled() {
	if (m_currentBinding) {
		return m_currentBinding->data.keyboard.altPressed;
	} else {
		return false;
	}
}

unsigned DigitalInputRemappingController::keyboardKeyIndex() {
	if (m_currentBinding) {
		return parent->keyboardVirtualCodeIndexFromId(m_currentBinding->data.keyboard.keyCode);
	} else {
		return false;
	}
}


bool DigitalInputRemappingController::keyboardUseScanCode() {
	if (m_currentBinding) {
		return m_currentBinding->data.keyboard.sendScanCode;
	} else {
		return false;
	}
}

int DigitalInputRemappingController::getButtonMaxCount() {
	return (int)vr::k_EButton_Max;
}

QString DigitalInputRemappingController::getButtonName(int id, bool withDefaults) {
	QString name = parent->openvrButtonToString(vr::k_unTrackedDeviceIndexInvalid, id);
	if (withDefaults) {
		switch (id) {
		case vr::k_EButton_SteamVR_Touchpad:
			name.append(" (TrackPad)");
			break;
		case vr::k_EButton_SteamVR_Trigger:
			name.append(" (Trigger)");
			break;
		default:
			break;
		}
	}
	return name;
}


/*void DigitalInputRemappingController::enableLongPress(bool enable, bool notify) {
	if (m_currentRemapping.longPressEnabled != enable) {
		m_currentRemapping.longPressEnabled = enable;
		if (notify) {
			emit longPressEnabledChanged(m_currentRemapping.longPressEnabled);
		}
	}
}

void DigitalInputRemappingController::setLongPressThreshold(unsigned value, bool notify) {
	if (m_currentRemapping.longPressThreshold != value) {
		m_currentRemapping.longPressThreshold = value;
		if (notify) {
			emit longPressThresholdChanged(m_currentRemapping.longPressThreshold);
		}
	}
}

void DigitalInputRemappingController::setLongPressImmediateRelease(unsigned value, bool notify) {
	if (m_currentRemapping.longPressImmediateRelease != value) {
		m_currentRemapping.longPressImmediateRelease = value;
		if (notify) {
			emit longPressImmediateReleaseChanged(m_currentRemapping.longPressImmediateRelease);
		}
	}
}

void DigitalInputRemappingController::enableDoublePress(bool enable, bool notify) {
	if (m_currentRemapping.doublePressEnabled != enable) {
		m_currentRemapping.doublePressEnabled = enable;
		if (notify) {
			emit doublePressEnabledChanged(m_currentRemapping.doublePressEnabled);
		}
	}
}

void DigitalInputRemappingController::setDoublePressThreshold(unsigned value, bool notify) {
	if (m_currentRemapping.doublePressThreshold != value) {
		m_currentRemapping.doublePressThreshold = value;
		if (notify) {
			emit doublePressThresholdChanged(m_currentRemapping.doublePressThreshold);
		}
	}
}

void DigitalInputRemappingController::setDoublePressImmediateRelease(unsigned value, bool notify) {
	if (m_currentRemapping.doublePressImmediateRelease != value) {
		m_currentRemapping.doublePressImmediateRelease = value;
		if (notify) {
			emit doublePressImmediateReleaseChanged(m_currentRemapping.doublePressImmediateRelease);
		}
	}
}

void DigitalInputRemappingController::enableTouchAsClick(bool enable, bool notify) {
	if (m_currentRemapping.touchAsClick != enable) {
		m_currentRemapping.touchAsClick = enable;
		if (notify) {
			emit touchAsClickChanged(m_currentRemapping.touchAsClick);
		}
	}

}*/

void DigitalInputRemappingController::finishConfigureBinding_Original() {
	m_currentBinding->type = vrinputemulator::DigitalBindingType::NoRemapping;
	memset(&m_currentBinding->data, 0, sizeof(m_currentBinding->data));
	m_currentBinding->toggleEnabled = false;
	m_currentBinding->toggleDelay = 0;
	m_currentBinding->autoTriggerEnabled = false;
	m_currentBinding->autoTriggerFrequency = 1;
	emit configureDigitalBindingFinished();
}

void DigitalInputRemappingController::finishConfigureBinding_Disabled() {
	m_currentBinding->type = vrinputemulator::DigitalBindingType::Disabled;
	memset(&m_currentBinding->data, 0, sizeof(m_currentBinding->data));
	m_currentBinding->toggleEnabled = false;
	m_currentBinding->toggleDelay = 0;
	m_currentBinding->autoTriggerEnabled = false;
	m_currentBinding->autoTriggerFrequency = 1;
	emit configureDigitalBindingFinished();
}

void DigitalInputRemappingController::finishConfigureBinding_OpenVR(int controllerId, int ButtonId, bool toggleMode, int toggleThreshold, bool autoTrigger, int triggerFrequency) {
	m_currentBinding->type = vrinputemulator::DigitalBindingType::OpenVR;
	memset(&m_currentBinding->data, 0, sizeof(m_currentBinding->data));
	if (controllerId < 0) {
		m_currentBinding->data.openvr.controllerId = vr::k_unTrackedDeviceIndexInvalid;
	} else {
		m_currentBinding->data.openvr.controllerId = controllerId;
	}
	m_currentBinding->data.openvr.buttonId = ButtonId;
	m_currentBinding->toggleEnabled = toggleMode;
	m_currentBinding->toggleDelay = toggleThreshold;
	m_currentBinding->autoTriggerEnabled = autoTrigger;
	m_currentBinding->autoTriggerFrequency = triggerFrequency;
	emit configureDigitalBindingFinished();
}

void DigitalInputRemappingController::finishConfigureBinding_keyboard(bool shiftPressed, bool ctrlPressed, bool altPressed, unsigned long keyIndex, bool useScanCode, bool toggleMode, int toggleThreshold, bool autoTrigger, int triggerFrequency) {
	m_currentBinding->type = vrinputemulator::DigitalBindingType::Keyboard;
	memset(&m_currentBinding->data, 0, sizeof(m_currentBinding->data));
	m_currentBinding->data.keyboard.shiftPressed = shiftPressed;
	m_currentBinding->data.keyboard.ctrlPressed = ctrlPressed;
	m_currentBinding->data.keyboard.altPressed = altPressed;
	m_currentBinding->data.keyboard.keyCode = parent->keyboardVirtualCodeIdFromIndex(keyIndex);
	m_currentBinding->data.keyboard.sendScanCode = useScanCode;
	m_currentBinding->toggleEnabled = toggleMode;
	m_currentBinding->toggleDelay = toggleThreshold;
	m_currentBinding->autoTriggerEnabled = autoTrigger;
	m_currentBinding->autoTriggerFrequency = triggerFrequency;
	emit configureDigitalBindingFinished();
}

void DigitalInputRemappingController::finishConfigureBinding_suspendRedirectMode() {
	m_currentBinding->type = vrinputemulator::DigitalBindingType::SuspendRedirectMode;
	memset(&m_currentBinding->data, 0, sizeof(m_currentBinding->data));
	m_currentBinding->toggleEnabled = false;
	m_currentBinding->toggleDelay = 0;
	m_currentBinding->autoTriggerEnabled = false;
	m_currentBinding->autoTriggerFrequency = 1;
	emit configureDigitalBindingFinished();
}


void DigitalInputRemappingController::finishConfigureBinding_toggleTouchpadEmulationFix() {
	m_currentBinding->type = vrinputemulator::DigitalBindingType::ToggleTouchpadEmulationFix;
	memset(&m_currentBinding->data, 0, sizeof(m_currentBinding->data));
	m_currentBinding->toggleEnabled = false;
	m_currentBinding->toggleDelay = 0;
	m_currentBinding->autoTriggerEnabled = false;
	m_currentBinding->autoTriggerFrequency = 1;
	emit configureDigitalBindingFinished();
}


} // namespace inputemulator
