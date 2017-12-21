#include "AnalogInputRemappingController.h"
#include "../overlaycontroller.h"

// application namespace
namespace inputemulator {

AnalogInputRemappingController::~AnalogInputRemappingController() {
}

void AnalogInputRemappingController::initStage1() {
}


void AnalogInputRemappingController::initStage2(OverlayController * parent, QQuickWindow * widget) {
	this->parent = parent;
	this->widget = widget;
}


void AnalogInputRemappingController::eventLoopTick(vr::TrackedDevicePose_t* /*devicePoses*/) {
	if (settingsUpdateCounter >= 50) {
		settingsUpdateCounter = 0;
	} else {
		settingsUpdateCounter++;
	}
}

void AnalogInputRemappingController::handleEvent(const vr::VREvent_t&) {
	/*switch (vrEvent.eventType) {
	default:
	break;
	}*/
}

void AnalogInputRemappingController::startConfigureRemapping(vrinputemulator::AnalogInputRemapping remapping, uint32_t deviceIndex, uint32_t deviceId, uint32_t axisId) {
	m_currentDeviceIndex = deviceIndex;
	m_currentDeviceId = deviceId;
	m_currentAxisId = axisId;
	m_currentRemapping = remapping;
}

int AnalogInputRemappingController::getAxisMaxCount() {
	return vr::k_unControllerStateAxisCount;
}

QString AnalogInputRemappingController::getAxisName(int id, bool withDefaults) {
	QString name("Axis");
	name.append(QString::number(id));
	if (withDefaults) {
		switch (id) {
		case 0:
			name.append(" (TrackPad)");
			break;
		case 1:
			name.append(" (Trigger)");
			break;
		default:
			break;
		}
	}
	return name;
}



int AnalogInputRemappingController::getBindingType() {
	if (m_currentRemapping.valid) {
		return (int)m_currentRemapping.binding.type;
	} else {
		return 0;
	}
}

int AnalogInputRemappingController::getBindingOpenVRControllerId() {
	if (m_currentRemapping.valid) {
		return m_currentRemapping.binding.data.openvr.controllerId;
	} else {
		return -1;
	}
}

int AnalogInputRemappingController::getBindingOpenVRAxisId() {
	if (m_currentRemapping.valid) {
		return m_currentRemapping.binding.data.openvr.axisId;
	} else {
		return -1;
	}
}

bool AnalogInputRemappingController::isBindingOpenVRXInverted() {
	if (m_currentRemapping.valid) {
		return m_currentRemapping.binding.invertXAxis;
	} else {
		return false;
	}
}

bool AnalogInputRemappingController::isBindingOpenVRYInverted() {
	if (m_currentRemapping.valid) {
		return m_currentRemapping.binding.invertYAxis;
	} else {
		return false;
	}
}

bool AnalogInputRemappingController::isBindingOpenVRAxesSwapped() {
	if (m_currentRemapping.valid) {
		return m_currentRemapping.binding.swapAxes;
	} else {
		return false;
	}
}

float AnalogInputRemappingController::getBindingDeadzoneLower() {
	if (m_currentRemapping.valid) {
		return m_currentRemapping.binding.lowerDeadzone;
	} else {
		return false;
	}
}

float AnalogInputRemappingController::getBindingDeadzoneUpper() {
	if (m_currentRemapping.valid) {
		return m_currentRemapping.binding.upperDeadzone;
	} else {
		return false;
	}
}

unsigned AnalogInputRemappingController::getBindingTouchpadEmulationMode() {
	if (m_currentRemapping.valid) {
		return m_currentRemapping.binding.touchpadEmulationMode;
	} else {
		return false;
	}
}

bool AnalogInputRemappingController::getBindingButtonPressDeadzoneFix() {
	if (m_currentRemapping.valid) {
		return m_currentRemapping.binding.buttonPressDeadzoneFix;
	} else {
		return false;
	}
}

void AnalogInputRemappingController::finishConfigure_Original(unsigned touchpadEmulationMode, bool updateOnButtonEvent) {
	m_currentRemapping.binding.type = vrinputemulator::AnalogBindingType::NoRemapping;
	m_currentRemapping.binding.touchpadEmulationMode = touchpadEmulationMode;
	m_currentRemapping.binding.buttonPressDeadzoneFix = updateOnButtonEvent;
	parent->deviceManipulationTabController.finishConfigureAnalogInputRemapping(m_currentDeviceIndex, m_currentAxisId);
}

void AnalogInputRemappingController::finishConfigure_Disabled() {
	m_currentRemapping.binding.type = vrinputemulator::AnalogBindingType::Disabled;
	parent->deviceManipulationTabController.finishConfigureAnalogInputRemapping(m_currentDeviceIndex, m_currentAxisId);
}

void AnalogInputRemappingController::finishConfigure_OpenVR(int controllerId, int axisId, bool invertXAxis, bool invertYAxis, bool swapAxes, float lowerDeadzone, float upperDeadzone, unsigned touchpadEmulationMode, bool updateOnButtonEvent) {
	m_currentRemapping.binding.type = vrinputemulator::AnalogBindingType::OpenVR;
	if (controllerId < 0) {
		m_currentRemapping.binding.data.openvr.controllerId = vr::k_unTrackedDeviceIndexInvalid;
	} else {
		m_currentRemapping.binding.data.openvr.controllerId = controllerId;
	}
	m_currentRemapping.binding.data.openvr.axisId = axisId;
	m_currentRemapping.binding.invertXAxis = invertXAxis;
	m_currentRemapping.binding.invertYAxis = invertYAxis;
	m_currentRemapping.binding.swapAxes = swapAxes;
	m_currentRemapping.binding.lowerDeadzone = lowerDeadzone;
	m_currentRemapping.binding.upperDeadzone = upperDeadzone;
	m_currentRemapping.binding.touchpadEmulationMode = touchpadEmulationMode;
	m_currentRemapping.binding.buttonPressDeadzoneFix = updateOnButtonEvent;
	parent->deviceManipulationTabController.finishConfigureAnalogInputRemapping(m_currentDeviceIndex, m_currentAxisId);
}


} // namespace inputemulator
