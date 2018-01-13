#include "DeviceManipulationTabController.h"
#include <QQuickWindow>
#include <QApplication>
#include "../overlaycontroller.h"
#include <openvr_math.h>
#include <vrinputemulator_types.h>
#include <ipc_protocol.h>
#include <chrono>

// application namespace
namespace inputemulator {

DeviceManipulationTabController::~DeviceManipulationTabController() {
	if (identifyThread.joinable()) {
		identifyThread.join();
	}
}


void DeviceManipulationTabController::initStage1() {
	reloadDeviceManipulationProfiles();
	reloadDeviceManipulationSettings();
}


void DeviceManipulationTabController::initStage2(OverlayController * parent, QQuickWindow * widget) {
	this->parent = parent;
	this->widget = widget;
	try {
		for (uint32_t id = 0; id < vr::k_unMaxTrackedDeviceCount; ++id) {
			auto deviceClass = vr::VRSystem()->GetTrackedDeviceClass(id);
			if (deviceClass != vr::TrackedDeviceClass_Invalid) {
				if (deviceClass == vr::TrackedDeviceClass_HMD || deviceClass == vr::TrackedDeviceClass_Controller || deviceClass == vr::TrackedDeviceClass_GenericTracker) {
					auto info = std::make_shared<DeviceInfo>();
					info->openvrId = id;
					info->deviceClass = deviceClass;
					char buffer[vr::k_unMaxPropertyStringSize];
					vr::ETrackedPropertyError pError = vr::TrackedProp_Success;
					vr::VRSystem()->GetStringTrackedDeviceProperty(id, vr::Prop_SerialNumber_String, buffer, vr::k_unMaxPropertyStringSize, &pError);
					if (pError == vr::TrackedProp_Success) {
						info->serial = std::string(buffer);
					} else {
						info->serial = std::string("<unknown serial>");
						LOG(ERROR) << "Could not get serial of device " << id;
					}

					try {
						vrinputemulator::DeviceInfo info2;
						parent->vrInputEmulator().getDeviceInfo(info->openvrId, info2);
						info->deviceMode = info2.deviceMode;
						info->deviceOffsetsEnabled = info2.offsetsEnabled;
						if (info->deviceMode == 2 || info->deviceMode == 3) {
							info->deviceStatus = info2.redirectSuspended ? 1 : 0;
						}
					} catch (std::exception& e) {
						LOG(ERROR) << "Exception caught while getting device info: " << e.what();
					}

					deviceInfos.push_back(info);
					LOG(INFO) << "Found device: id " << info->openvrId << ", class " << info->deviceClass << ", serial " << info->serial;
				}
				maxValidDeviceId = id;
			}
		}
		emit deviceCountChanged((unsigned)deviceInfos.size());
	} catch (const std::exception& e) {
		LOG(ERROR) << "Could not get device infos: " << e.what();
	}
}


void DeviceManipulationTabController::eventLoopTick(vr::TrackedDevicePose_t* devicePoses) {
	if (settingsUpdateCounter >= 50) {
		settingsUpdateCounter = 0;
		if (parent->isDashboardVisible() || parent->isDesktopMode()) {
			unsigned i = 0;
			for (auto info : deviceInfos) {
				bool hasDeviceInfoChanged = updateDeviceInfo(i);
				unsigned status = devicePoses[info->openvrId].bDeviceIsConnected ? 0 : 1;
				if (info->deviceMode == 0 && info->deviceStatus != status) {
					info->deviceStatus = status;
					hasDeviceInfoChanged = true;
				}
				if (hasDeviceInfoChanged) {
					emit deviceInfoChanged(i);
				}
				++i;
			}
			bool newDeviceAdded = false;
			for (uint32_t id = maxValidDeviceId + 1; id < vr::k_unMaxTrackedDeviceCount; ++id) {
				auto deviceClass = vr::VRSystem()->GetTrackedDeviceClass(id);
				if (deviceClass != vr::TrackedDeviceClass_Invalid) {
					if (deviceClass == vr::TrackedDeviceClass_Controller || deviceClass == vr::TrackedDeviceClass_GenericTracker) {
						auto info = std::make_shared<DeviceInfo>();
						info->openvrId = id;
						info->deviceClass = deviceClass;
						char buffer[vr::k_unMaxPropertyStringSize];
						vr::ETrackedPropertyError pError = vr::TrackedProp_Success;
						vr::VRSystem()->GetStringTrackedDeviceProperty(id, vr::Prop_SerialNumber_String, buffer, vr::k_unMaxPropertyStringSize, &pError);
						if (pError == vr::TrackedProp_Success) {
							info->serial = std::string(buffer);
						} else {
							info->serial = std::string("<unknown serial>");
							LOG(ERROR) << "Could not get serial of device " << id;
						}

						try {
							vrinputemulator::DeviceInfo info2;
							parent->vrInputEmulator().getDeviceInfo(info->openvrId, info2);
							info->deviceMode = info2.deviceMode;
							info->deviceOffsetsEnabled = info2.offsetsEnabled;
							if (info->deviceMode == 2 || info->deviceMode == 3) {
								info->deviceStatus = info2.redirectSuspended ? 1 : 0;
							}
						} catch (std::exception& e) {
							LOG(ERROR) << "Exception caught while getting device info: " << e.what();
						}

						deviceInfos.push_back(info);
						LOG(INFO) << "Found device: id " << info->openvrId << ", class " << info->deviceClass << ", serial " << info->serial;
						newDeviceAdded = true;
					}
					maxValidDeviceId = id;
				}
			}
			if (newDeviceAdded) {
				emit deviceCountChanged((unsigned)deviceInfos.size());
			}
		}
	} else {
		settingsUpdateCounter++;
	}
}

void DeviceManipulationTabController::handleEvent(const vr::VREvent_t&) {
	/*switch (vrEvent.eventType) {
		default:
			break;
	}*/
}

unsigned  DeviceManipulationTabController::getDeviceCount() {
	return (unsigned)deviceInfos.size();
}

QString DeviceManipulationTabController::getDeviceSerial(unsigned index) {
	if (index < deviceInfos.size()) {
		return QString::fromStdString(deviceInfos[index]->serial);
	} else {
		return QString("<ERROR>");
	}
}

unsigned DeviceManipulationTabController::getDeviceId(unsigned index) {
	if (index < deviceInfos.size()) {
		return (int)deviceInfos[index]->openvrId;
	} else {
		return vr::k_unTrackedDeviceIndexInvalid;
	}
}

int DeviceManipulationTabController::getDeviceClass(unsigned index) {
	if (index < deviceInfos.size()) {
		return (int)deviceInfos[index]->deviceClass;
	} else {
		return -1;
	}
}

int DeviceManipulationTabController::getDeviceState(unsigned index) {
	if (index < deviceInfos.size()) {
		return (int)deviceInfos[index]->deviceStatus;
	} else {
		return -1;
	}
}

int DeviceManipulationTabController::getDeviceMode(unsigned index) {
	if (index < deviceInfos.size()) {
		return (int)deviceInfos[index]->deviceMode;
	} else {
		return -1;
	}
}

int DeviceManipulationTabController::getDeviceModeRefDeviceIndex(unsigned index) {
	int retval = (int)vr::k_unTrackedDeviceIndexInvalid;
	if (index < deviceInfos.size()) {
		for (unsigned i = 0; i < deviceInfos.size(); i++) {
			if (deviceInfos[i]->openvrId == deviceInfos[index]->refDeviceId) {
				retval = i;
				break;
			}
		}
	}
	return retval;
}

bool DeviceManipulationTabController::deviceOffsetsEnabled(unsigned index) {
	if (index < deviceInfos.size()) {
		return deviceInfos[index]->deviceOffsetsEnabled;
	} else {
		return false;
	}
}

double DeviceManipulationTabController::getWorldFromDriverRotationOffset(unsigned index, unsigned axis) {
	if (index < deviceInfos.size() && axis < 3) {
		return deviceInfos[index]->worldFromDriverRotationOffset.v[axis];
	} else {
		return 0.0;
	}
}

double DeviceManipulationTabController::getWorldFromDriverTranslationOffset(unsigned index, unsigned axis) {
	if (index < deviceInfos.size() && axis < 3) {
		return deviceInfos[index]->worldFromDriverTranslationOffset.v[axis];
	} else {
		return 0.0;
	}
}

double DeviceManipulationTabController::getDriverFromHeadRotationOffset(unsigned index, unsigned axis) {
	if (index < deviceInfos.size() && axis < 3) {
		return deviceInfos[index]->driverFromHeadRotationOffset.v[axis];
	} else {
		return 0.0;
	}
}

double DeviceManipulationTabController::getDriverFromHeadTranslationOffset(unsigned index, unsigned axis) {
	if (index < deviceInfos.size() && axis < 3) {
		return deviceInfos[index]->driverFromHeadTranslationOffset.v[axis];
	} else {
		return 0.0;
	}
}

double DeviceManipulationTabController::getDriverRotationOffset(unsigned index, unsigned axis) {
	if (index < deviceInfos.size() && axis < 3) {
		return deviceInfos[index]->deviceRotationOffset.v[axis];
	} else {
		return 0.0;
	}
}

double DeviceManipulationTabController::getDriverTranslationOffset(unsigned index, unsigned axis) {
	if (index < deviceInfos.size() && axis < 3) {
		return deviceInfos[index]->deviceTranslationOffset.v[axis];
	} else {
		return 0.0;
	}
}

unsigned DeviceManipulationTabController::getMotionCompensationVelAccMode() {
	return (unsigned)motionCompensationVelAccMode;
}

double DeviceManipulationTabController::getMotionCompensationKalmanProcessNoise() {
	return motionCompensationKalmanProcessNoise;
}

double DeviceManipulationTabController::getMotionCompensationKalmanObservationNoise() {
	return motionCompensationKalmanObservationNoise;
}

unsigned DeviceManipulationTabController::getMotionCompensationMovingAverageWindow() {
	return motionCompensationMovingAverageWindow;
}


#define DEVICEMANIPULATIONSETTINGS_GETTRANSLATIONVECTOR(name) { \
	double valueX = settings->value(#name ## "_x", 0.0).toDouble(); \
	double valueY = settings->value(#name ## "_y", 0.0).toDouble(); \
	double valueZ = settings->value(#name ## "_z", 0.0).toDouble(); \
	entry.name = { valueX, valueY, valueZ }; \
}

#define DEVICEMANIPULATIONSETTINGS_GETROTATIONVECTOR(name) { \
	double valueY = settings->value(#name ## "_yaw", 0.0).toDouble(); \
	double valueP = settings->value(#name ## "_pitch", 0.0).toDouble(); \
	double valueR = settings->value(#name ## "_roll", 0.0).toDouble(); \
	entry.name = { valueY, valueP, valueR }; \
}

void DeviceManipulationTabController::reloadDeviceManipulationSettings() {
	auto settings = OverlayController::appSettings();
	settings->beginGroup("deviceManipulationSettings");
	motionCompensationVelAccMode = (vrinputemulator::MotionCompensationVelAccMode)settings->value("motionCompensationVelAccMode", 0).toUInt();
	motionCompensationKalmanProcessNoise = settings->value("motionCompensationKalmanProcessNoise", 0.1).toDouble();
	motionCompensationKalmanObservationNoise = settings->value("motionCompensationKalmanObservationNoise", 0.1).toDouble();
	motionCompensationMovingAverageWindow = settings->value("motionCompensationMovingAverageWindow", 3).toUInt();
	settings->endGroup();
}

void DeviceManipulationTabController::reloadDeviceManipulationProfiles() {
	deviceManipulationProfiles.clear();
	auto settings = OverlayController::appSettings();
	settings->beginGroup("deviceManipulationSettings");
	auto profileCount = settings->beginReadArray("deviceManipulationProfiles");
	for (int i = 0; i < profileCount; i++) {
		settings->setArrayIndex(i);
		deviceManipulationProfiles.emplace_back();
		auto& entry = deviceManipulationProfiles[i];
		entry.profileName = settings->value("profileName").toString().toStdString();
		entry.includesDeviceOffsets = settings->value("includesDeviceOffsets", false).toBool();
		if (entry.includesDeviceOffsets) {
			entry.deviceOffsetsEnabled = settings->value("deviceOffsetsEnabled", false).toBool();
			DEVICEMANIPULATIONSETTINGS_GETTRANSLATIONVECTOR(worldFromDriverTranslationOffset);
			DEVICEMANIPULATIONSETTINGS_GETROTATIONVECTOR(worldFromDriverRotationOffset);
			DEVICEMANIPULATIONSETTINGS_GETTRANSLATIONVECTOR(driverFromHeadTranslationOffset);
			DEVICEMANIPULATIONSETTINGS_GETROTATIONVECTOR(driverFromHeadRotationOffset);
			DEVICEMANIPULATIONSETTINGS_GETTRANSLATIONVECTOR(driverTranslationOffset);
			DEVICEMANIPULATIONSETTINGS_GETROTATIONVECTOR(driverRotationOffset);
		}

		entry.includesInputRemapping = settings->value("includesInputRemapping", false).toBool();
		if (entry.includesInputRemapping) {

			auto digitalRemappings = settings->value("digitalInputRemappings", false).toMap();

			auto loadDigitalBinding = [](vrinputemulator::DigitalBinding& binding, QString& serial, QVariantMap& data) {
				binding.type = (vrinputemulator::DigitalBindingType)data["type"].toUInt();
				if (binding.type == vrinputemulator::DigitalBindingType::OpenVR) {
					if (data.contains("controllerSerial")) {
						serial = data["controllerSerial"].toString();
					}
					binding.data.openvr.buttonId = data["buttonId"].toUInt();
				} else if (binding.type == vrinputemulator::DigitalBindingType::Keyboard) {
					binding.data.keyboard.shiftPressed = data["shiftPressed"].toBool();
					binding.data.keyboard.altPressed = data["altPressed"].toBool();
					binding.data.keyboard.ctrlPressed = data["ctrlPressed"].toBool();
					binding.data.keyboard.keyCode = data["keyCode"].toUInt();
					if (data.contains("sendScanCode")) {
						binding.data.keyboard.sendScanCode = true;
					} else {
						binding.data.keyboard.sendScanCode = data["sendScanCode"].toBool();
					}
				}
				binding.toggleEnabled = data["toggleEnabled"].toBool();
				binding.toggleDelay = data["toggleDelay"].toUInt();
				binding.autoTriggerEnabled = data["autoTriggerEnabled"].toBool();
				binding.autoTriggerFrequency = data["autoTriggerFrequency"].toUInt();
			};

			for (auto key : digitalRemappings.keys()) {
				auto r = digitalRemappings[key].toMap();
				DigitalInputRemappingProfile p;
				loadDigitalBinding(p.remapping.binding, p.normalBindingControllerSerial, r["binding"].toMap());
				p.remapping.touchAsClick = r.contains("touchAsClick") ? r["touchAsClick"].toBool() : false;
				p.remapping.longPressEnabled = r.contains("longPressEnabled") ? r["longPressEnabled"].toBool() : false;
				if (p.remapping.longPressEnabled) {
					loadDigitalBinding(p.remapping.longPressBinding, p.longBindingControllerSerial, r["longPressBinding"].toMap());
					p.remapping.longPressThreshold = r.contains("longPressThreshold") ? r["longPressThreshold"].toUInt() : 1000u;
					p.remapping.longPressImmediateRelease = r.contains("longPressImmediateRelease") ? r["longPressImmediateRelease"].toBool() : false;
				}
				p.remapping.doublePressEnabled = r.contains("doublePressEnabled") ? r["doublePressEnabled"].toBool() : false;
				if (p.remapping.doublePressEnabled) {
					loadDigitalBinding(p.remapping.doublePressBinding, p.doubleBindingControllerSerial, r["doublePressBinding"].toMap());
					p.remapping.doublePressThreshold = r.contains("doublePressThreshold") ? r["doublePressThreshold"].toUInt() : 1000u;
					p.remapping.doublePressImmediateRelease = r.contains("doublePressImmediateRelease") ? r["doublePressImmediateRelease"].toBool() : false;
				}
				entry.digitalRemappingProfiles[key.toInt()] = p;
			}

			auto analogRemappings = settings->value("analogInputRemappings", false).toMap();

			for (auto key : analogRemappings.keys()) {
				auto r = analogRemappings[key].toMap();
				AnalogInputRemappingProfile p;
				p.remapping.binding.type = (vrinputemulator::AnalogBindingType)(r.contains("type") ? r["type"].toUInt() : (unsigned)vrinputemulator::AnalogBindingType::NoRemapping);
				if (p.remapping.binding.type == vrinputemulator::AnalogBindingType::OpenVR) {
					p.remapping.binding.data.openvr.axisId = r["axisId"].toUInt();
					if (r.contains("controllerSerial")) {
						p.controllerSerial = r["controllerSerial"].toString();
					}
				}
				p.remapping.binding.invertXAxis = r["invertXAxis"].toBool();
				p.remapping.binding.invertYAxis = r["invertYAxis"].toBool();
				p.remapping.binding.swapAxes = r["swapAxes"].toBool();
				p.remapping.binding.lowerDeadzone = r["lowerDeadzone"].toFloat();
				p.remapping.binding.upperDeadzone = r["upperDeadzone"].toFloat();
				p.remapping.binding.buttonPressDeadzoneFix = r["buttonPressDeadzoneFix"].toBool();
				p.remapping.binding.touchpadEmulationMode = r["touchpadEmulationMode"].toUInt();
				entry.analogRemappingProfiles[key.toInt()] = p;
			}
		}
	}
	settings->endArray();
	settings->endGroup();
}

void DeviceManipulationTabController::saveDeviceManipulationSettings() {
	auto settings = OverlayController::appSettings();
	settings->beginGroup("deviceManipulationSettings");
	settings->setValue("motionCompensationVelAccMode", (unsigned)motionCompensationVelAccMode);
	settings->setValue("motionCompensationKalmanProcessNoise", motionCompensationKalmanProcessNoise);
	settings->setValue("motionCompensationKalmanObservationNoise", motionCompensationKalmanObservationNoise);
	settings->setValue("motionCompensationMovingAverageWindow", motionCompensationMovingAverageWindow);
	settings->endGroup();
	settings->sync();
}


#define DEVICEMANIPULATIONSETTINGS_WRITETRANSLATIONVECTOR(name) { \
	auto& vec = p.name; \
	settings->setValue(#name ## "_x", vec.v[0]); \
	settings->setValue(#name ## "_y", vec.v[1]); \
	settings->setValue(#name ## "_z", vec.v[2]); \
}


#define DEVICEMANIPULATIONSETTINGS_WRITEROTATIONVECTOR(name) { \
	auto& vec = p.name; \
	settings->setValue(#name ## "_yaw", vec.v[0]); \
	settings->setValue(#name ## "_pitch", vec.v[1]); \
	settings->setValue(#name ## "_roll", vec.v[2]); \
}


void DeviceManipulationTabController::saveDeviceManipulationProfiles() {
	auto settings = OverlayController::appSettings();
	settings->beginGroup("deviceManipulationSettings");
	settings->beginWriteArray("deviceManipulationProfiles");
	unsigned i = 0;

	auto saveDigitalBinding = [this](const vrinputemulator::DigitalBinding& binding, const QString& serial)->QVariantMap {
		QVariantMap data;
		data["type"] = (int)binding.type;
		if (binding.type == vrinputemulator::DigitalBindingType::OpenVR) {
			data["buttonId"] = binding.data.keyboard.keyCode;
			if (!serial.isEmpty()) {
				data["controllerSerial"] = serial;
			}
		} else if (binding.type == vrinputemulator::DigitalBindingType::Keyboard) {
			data["shiftPressed"] = binding.data.keyboard.shiftPressed;
			data["altPressed"] = binding.data.keyboard.altPressed;
			data["ctrlPressed"] = binding.data.keyboard.ctrlPressed;
			data["keyCode"] = binding.data.keyboard.keyCode;
			data["sendScanCode"] = binding.data.keyboard.sendScanCode;
		}
		data["toggleEnabled"] = binding.toggleEnabled;
		data["autoTriggerEnabled"] = binding.autoTriggerEnabled;
		data["autoTriggerFrequency"] = binding.autoTriggerFrequency;
		return data;
	};

	for (auto& p : deviceManipulationProfiles) {
		settings->setArrayIndex(i);
		settings->setValue("profileName", QString::fromStdString(p.profileName));
		settings->setValue("includesDeviceOffsets", p.includesDeviceOffsets);
		if (p.includesDeviceOffsets) {
			settings->setValue("deviceOffsetsEnabled", p.deviceOffsetsEnabled);
			DEVICEMANIPULATIONSETTINGS_WRITETRANSLATIONVECTOR(worldFromDriverTranslationOffset);
			DEVICEMANIPULATIONSETTINGS_WRITEROTATIONVECTOR(worldFromDriverRotationOffset);
			DEVICEMANIPULATIONSETTINGS_WRITETRANSLATIONVECTOR(driverFromHeadTranslationOffset);
			DEVICEMANIPULATIONSETTINGS_WRITEROTATIONVECTOR(driverFromHeadRotationOffset);
			DEVICEMANIPULATIONSETTINGS_WRITETRANSLATIONVECTOR(driverTranslationOffset);
			DEVICEMANIPULATIONSETTINGS_WRITEROTATIONVECTOR(driverRotationOffset);
		}
		settings->setValue("includesInputRemapping", p.includesInputRemapping);
		if (p.includesInputRemapping) {
			QVariantMap digitalProfiles;
			for (auto dp : p.digitalRemappingProfiles) {
				QVariantMap profile;
				profile["binding"] = saveDigitalBinding(dp.second.remapping.binding, dp.second.normalBindingControllerSerial);
				profile["touchAsClick"] = dp.second.remapping.touchAsClick;
				profile["longPressEnabled"] = dp.second.remapping.longPressEnabled;
				if (dp.second.remapping.longPressEnabled) {
					profile["longPressBinding"] =  saveDigitalBinding(dp.second.remapping.longPressBinding, dp.second.longBindingControllerSerial);
					profile["longPressThreshold"] = dp.second.remapping.longPressThreshold;
					profile["longPressImmediateRelease"] = dp.second.remapping.longPressImmediateRelease;
				}
				profile["doublePressEnabled"] = dp.second.remapping.doublePressEnabled;
				if (dp.second.remapping.doublePressEnabled) {
					profile["doublePressBinding"] = saveDigitalBinding(dp.second.remapping.doublePressBinding, dp.second.doubleBindingControllerSerial);
					profile["doublePressThreshold"] = dp.second.remapping.doublePressThreshold;
					profile["doublePressImmediateRelease"] = dp.second.remapping.doublePressImmediateRelease;
				}
				digitalProfiles[QString::number(dp.first)] = profile;
			}
			settings->setValue("digitalInputRemappings", digitalProfiles);
			QVariantMap analogProfiles;
			for (unsigned i2 = 0; i2 < 5; i2++) {
				auto& ap = p.analogRemappingProfiles[i2];
				if (ap.remapping.valid) {
					QVariantMap profile;
					profile["type"] = (int)ap.remapping.binding.type;
					if (ap.remapping.binding.type == vrinputemulator::AnalogBindingType::OpenVR) {
						profile["axisId"] = ap.remapping.binding.data.openvr.axisId;
						if (!ap.controllerSerial.isEmpty()) {
							profile["controllerSerial"] = ap.controllerSerial;
						}
					}
					profile["invertXAxis"] = ap.remapping.binding.invertXAxis;
					profile["invertYAxis"] = ap.remapping.binding.invertYAxis;
					profile["lowerDeadzone"] = ap.remapping.binding.lowerDeadzone;
					profile["upperDeadzone"] = ap.remapping.binding.upperDeadzone;
					profile["swapAxes"] = ap.remapping.binding.swapAxes;
					profile["buttonPressDeadzoneFix"] = ap.remapping.binding.buttonPressDeadzoneFix;
					profile["touchpadEmulationMode"] = ap.remapping.binding.touchpadEmulationMode;
					analogProfiles[QString::number(i2)] = profile;
				}
			}
			settings->setValue("analogInputRemappings", analogProfiles);
		}
		i++;
	}
	settings->endArray();
	settings->endGroup();
	settings->sync();
}

unsigned DeviceManipulationTabController::getDeviceManipulationProfileCount() {
	return (unsigned)deviceManipulationProfiles.size();
}

QString DeviceManipulationTabController::getDeviceManipulationProfileName(unsigned index) {
	if (index >= deviceManipulationProfiles.size()) {
		return QString();
	} else {
		return QString::fromStdString(deviceManipulationProfiles[index].profileName);
	}
}

void DeviceManipulationTabController::addDeviceManipulationProfile(QString name, unsigned deviceIndex, bool includesDeviceOffsets, bool includesInputRemapping) {
	if (deviceIndex >= deviceInfos.size()) {
		return;
	}
	auto device = deviceInfos[deviceIndex];
	DeviceManipulationProfile* profile = nullptr;
	for (auto& p : deviceManipulationProfiles) {
		if (p.profileName.compare(name.toStdString()) == 0) {
			profile = &p;
			break;
		}
	}
	if (!profile) {
		auto i = deviceManipulationProfiles.size();
		deviceManipulationProfiles.emplace_back();
		profile = &deviceManipulationProfiles[i];
	}
	profile->profileName = name.toStdString();
	profile->includesDeviceOffsets = includesDeviceOffsets;
	if (includesDeviceOffsets) {
		profile->deviceOffsetsEnabled = device->deviceOffsetsEnabled;
		profile->worldFromDriverTranslationOffset = device->worldFromDriverTranslationOffset;
		profile->worldFromDriverRotationOffset = device->worldFromDriverRotationOffset;
		profile->driverFromHeadTranslationOffset = device->driverFromHeadTranslationOffset;
		profile->driverFromHeadRotationOffset = device->driverFromHeadRotationOffset;
		profile->driverTranslationOffset = device->deviceTranslationOffset;
		profile->driverRotationOffset = device->deviceRotationOffset;
	}
	profile->includesInputRemapping = includesInputRemapping;
	if (includesInputRemapping) {
		try {
			auto _getDeviceSerialFromId = [&](uint32_t deviceId)->std::string {
				std::string retval;
				for (auto& i : deviceInfos) {
					if (i->openvrId == deviceId) {
						retval = i->serial;
						break;
					}
				}
				return retval;
			};
			for (unsigned i = 0; i < vr::k_EButton_Max; i++) {
				auto r = parent->vrInputEmulator().getDigitalInputRemapping(device->openvrId, i);
				if (r.valid) {
					auto& p = profile->digitalRemappingProfiles[i];
					p.remapping = r;
					if ( p.remapping.binding.type == vrinputemulator::DigitalBindingType::OpenVR
							&& p.remapping.binding.data.openvr.controllerId != vr::k_unTrackedDeviceIndexInvalid
							&& p.remapping.binding.data.openvr.controllerId != device->openvrId ) {
						p.normalBindingControllerSerial = QString::fromStdString(_getDeviceSerialFromId(p.remapping.binding.data.openvr.controllerId));
					}
					if ( p.remapping.longPressEnabled && p.remapping.longPressBinding.type == vrinputemulator::DigitalBindingType::OpenVR
							&& p.remapping.longPressBinding.data.openvr.controllerId != vr::k_unTrackedDeviceIndexInvalid
							&& p.remapping.longPressBinding.data.openvr.controllerId != device->openvrId) {
						p.longBindingControllerSerial = QString::fromStdString(_getDeviceSerialFromId(p.remapping.longPressBinding.data.openvr.controllerId));
					}
					if (p.remapping.doublePressEnabled && p.remapping.doublePressBinding.type == vrinputemulator::DigitalBindingType::OpenVR
							&& p.remapping.doublePressBinding.data.openvr.controllerId != vr::k_unTrackedDeviceIndexInvalid
							&& p.remapping.doublePressBinding.data.openvr.controllerId != device->openvrId) {
						p.doubleBindingControllerSerial = QString::fromStdString(_getDeviceSerialFromId(p.remapping.doublePressBinding.data.openvr.controllerId));
					}
				}
			}
			for (unsigned i = 0; i < 5; i++) {
				auto r = parent->vrInputEmulator().getAnalogInputRemapping(device->openvrId, i);
				auto& p = profile->analogRemappingProfiles[i];
				p.remapping.valid = r.valid;
				if (r.valid) {
					p.remapping.binding.type = r.binding.type;
					if (r.binding.type == vrinputemulator::AnalogBindingType::OpenVR) {
						p.remapping.binding.data.openvr.axisId = r.binding.data.openvr.axisId;
						if (r.binding.data.openvr.controllerId != vr::k_unTrackedDeviceIndexInvalid && r.binding.data.openvr.controllerId != device->openvrId) {
							p.controllerSerial = QString::fromStdString(_getDeviceSerialFromId(r.binding.data.openvr.controllerId));
						}
					}
					p.remapping.binding.invertXAxis = r.binding.invertXAxis;
					p.remapping.binding.invertYAxis = r.binding.invertYAxis;
					p.remapping.binding.lowerDeadzone = r.binding.lowerDeadzone;
					p.remapping.binding.upperDeadzone = r.binding.upperDeadzone;
					p.remapping.binding.swapAxes = r.binding.swapAxes;
					p.remapping.binding.buttonPressDeadzoneFix = r.binding.buttonPressDeadzoneFix;
					p.remapping.binding.touchpadEmulationMode = r.binding.touchpadEmulationMode;
				}
			}
		} catch (const std::exception& e) {
			LOG(ERROR) << "Exception while adding a new digital input remapping profile: " << e.what();
		}
	}
	saveDeviceManipulationProfiles();
	OverlayController::appSettings()->sync();
	emit deviceManipulationProfilesChanged();
}

void DeviceManipulationTabController::applyDeviceManipulationProfile(unsigned index, unsigned deviceIndex) {
	if (index < deviceManipulationProfiles.size()) {
		if (deviceIndex >= deviceInfos.size()) {
			return;
		}
		auto device = deviceInfos[deviceIndex];
		auto& profile = deviceManipulationProfiles[index];
		if (profile.includesDeviceOffsets) {
			setWorldFromDriverRotationOffset(deviceIndex, profile.worldFromDriverRotationOffset.v[0], profile.worldFromDriverRotationOffset.v[1], profile.worldFromDriverRotationOffset.v[2], false);
			setWorldFromDriverTranslationOffset(deviceIndex, profile.worldFromDriverTranslationOffset.v[0], profile.worldFromDriverTranslationOffset.v[1], profile.worldFromDriverTranslationOffset.v[2], false);
			setDriverFromHeadRotationOffset(deviceIndex, profile.driverFromHeadRotationOffset.v[0], profile.driverFromHeadRotationOffset.v[1], profile.driverFromHeadRotationOffset.v[2], false);
			setDriverFromHeadTranslationOffset(deviceIndex, profile.driverFromHeadTranslationOffset.v[0], profile.driverFromHeadTranslationOffset.v[1], profile.driverFromHeadTranslationOffset.v[2], false);
			setDriverRotationOffset(deviceIndex, profile.driverRotationOffset.v[0], profile.driverRotationOffset.v[1], profile.driverRotationOffset.v[2], false);
			setDriverTranslationOffset(deviceIndex, profile.driverTranslationOffset.v[0], profile.driverTranslationOffset.v[1], profile.driverTranslationOffset.v[2], false);
			enableDeviceOffsets(deviceIndex, profile.deviceOffsetsEnabled, false);
			updateDeviceInfo(deviceIndex);
		}
		if (profile.includesInputRemapping) {
			auto _getDeviceIdFromSerial = [&](const QString& serial)->uint32_t {
				uint32_t retval = vr::k_unTrackedDeviceIndexInvalid;
				for (auto i : deviceInfos) {
					if (i->serial.compare(serial.toStdString()) == 0) {
						retval = i->openvrId;
						break;
					}
				}
				return retval;
			};
			for (unsigned i = 0; i < vr::k_EButton_Max; i++) {
				auto it = profile.digitalRemappingProfiles.find(i);
				if (it == profile.digitalRemappingProfiles.end()) {
					parent->vrInputEmulator().setDigitalInputRemapping(device->openvrId, i, vrinputemulator::DigitalInputRemapping());
				} else {
					auto& p = *it;

					// We need to correct some potentially false data to not mess up the button state machine
					if (p.second.remapping.binding.type == vrinputemulator::DigitalBindingType::NoRemapping 
							|| p.second.remapping.binding.type == vrinputemulator::DigitalBindingType::Disabled
							|| p.second.remapping.binding.type == vrinputemulator::DigitalBindingType::SuspendRedirectMode
							|| p.second.remapping.binding.type == vrinputemulator::DigitalBindingType::ToggleTouchpadEmulationFix) {
						p.second.remapping.binding.toggleEnabled = false;
						p.second.remapping.binding.autoTriggerEnabled = false;
					}
					if (p.second.remapping.doublePressEnabled && (p.second.remapping.doublePressBinding.type == vrinputemulator::DigitalBindingType::NoRemapping
						|| p.second.remapping.doublePressBinding.type == vrinputemulator::DigitalBindingType::Disabled
						|| p.second.remapping.doublePressBinding.type == vrinputemulator::DigitalBindingType::SuspendRedirectMode
						|| p.second.remapping.doublePressBinding.type == vrinputemulator::DigitalBindingType::ToggleTouchpadEmulationFix)) {
						p.second.remapping.doublePressBinding.toggleEnabled = false;
						p.second.remapping.doublePressBinding.autoTriggerEnabled = false;
					}
					if (p.second.remapping.longPressEnabled && (p.second.remapping.longPressBinding.type == vrinputemulator::DigitalBindingType::NoRemapping
						|| p.second.remapping.longPressBinding.type == vrinputemulator::DigitalBindingType::Disabled
						|| p.second.remapping.longPressBinding.type == vrinputemulator::DigitalBindingType::SuspendRedirectMode
						|| p.second.remapping.longPressBinding.type == vrinputemulator::DigitalBindingType::ToggleTouchpadEmulationFix)) {
						p.second.remapping.longPressBinding.toggleEnabled = false;
						p.second.remapping.longPressBinding.autoTriggerEnabled = false;
					}

					if (p.second.remapping.binding.type == vrinputemulator::DigitalBindingType::OpenVR) {
						if (!p.second.normalBindingControllerSerial.isEmpty()) {
							p.second.remapping.binding.data.openvr.controllerId = _getDeviceIdFromSerial(p.second.normalBindingControllerSerial);
						} else {
							p.second.remapping.binding.data.openvr.controllerId = vr::k_unTrackedDeviceIndexInvalid;
						}
					}
					if (p.second.remapping.longPressBinding.type == vrinputemulator::DigitalBindingType::OpenVR) {
						if (!p.second.longBindingControllerSerial.isEmpty()) {
							p.second.remapping.longPressBinding.data.openvr.controllerId = _getDeviceIdFromSerial(p.second.longBindingControllerSerial);
						} else {
							p.second.remapping.longPressBinding.data.openvr.controllerId = vr::k_unTrackedDeviceIndexInvalid;
						}
					}
					if (p.second.remapping.doublePressBinding.type == vrinputemulator::DigitalBindingType::OpenVR) {
						if (!p.second.doubleBindingControllerSerial.isEmpty()) {
							p.second.remapping.doublePressBinding.data.openvr.controllerId = _getDeviceIdFromSerial(p.second.doubleBindingControllerSerial);
						} else {
							p.second.remapping.doublePressBinding.data.openvr.controllerId = vr::k_unTrackedDeviceIndexInvalid;
						}
					}
					parent->vrInputEmulator().setDigitalInputRemapping(device->openvrId, p.first, p.second.remapping);
				}
			}
			for (unsigned i = 0; i < 5; i++) {
				if (profile.analogRemappingProfiles[i].remapping.valid 
						&& profile.analogRemappingProfiles[i].remapping.binding.type == vrinputemulator::AnalogBindingType::OpenVR 
						&& !profile.analogRemappingProfiles[i].controllerSerial.isEmpty()) {
					profile.analogRemappingProfiles[i].remapping.binding.data.openvr.controllerId = _getDeviceIdFromSerial(profile.analogRemappingProfiles[i].controllerSerial);
				}
				parent->vrInputEmulator().setAnalogInputRemapping(device->openvrId, i, profile.analogRemappingProfiles[i].remapping);
			}
		}
		emit deviceInfoChanged(deviceIndex);
	}
}

void DeviceManipulationTabController::deleteDeviceManipulationProfile(unsigned index) {
	if (index < deviceManipulationProfiles.size()) {
		auto pos = deviceManipulationProfiles.begin() + index;
		deviceManipulationProfiles.erase(pos);
		saveDeviceManipulationProfiles();
		OverlayController::appSettings()->sync();
		emit deviceManipulationProfilesChanged();
	}
}

void DeviceManipulationTabController::setMotionCompensationVelAccMode(unsigned mode, bool notify) {
	vrinputemulator::MotionCompensationVelAccMode newMode = (vrinputemulator::MotionCompensationVelAccMode)mode;
	if (motionCompensationVelAccMode != newMode) {
		motionCompensationVelAccMode = newMode;
		parent->vrInputEmulator().setMotionVelAccCompensationMode(newMode);
		saveDeviceManipulationSettings();
		if (notify) {
			emit motionCompensationVelAccModeChanged(mode);
		}
	}
}

void DeviceManipulationTabController::setMotionCompensationKalmanProcessNoise(double variance, bool notify) {
	if (motionCompensationKalmanProcessNoise != variance) {
		motionCompensationKalmanProcessNoise = variance;
		parent->vrInputEmulator().setMotionCompensationKalmanProcessNoise(motionCompensationKalmanProcessNoise);
		saveDeviceManipulationSettings();
		if (notify) {
			emit motionCompensationKalmanProcessNoiseChanged(motionCompensationKalmanProcessNoise);
		}
	}
}

void DeviceManipulationTabController::setMotionCompensationKalmanObservationNoise(double variance, bool notify) {
	if (motionCompensationKalmanObservationNoise != variance) {
		motionCompensationKalmanObservationNoise = variance;
		parent->vrInputEmulator().setMotionCompensationKalmanObservationNoise(motionCompensationKalmanObservationNoise);
		saveDeviceManipulationSettings();
		if (notify) {
			emit motionCompensationKalmanObservationNoiseChanged(motionCompensationKalmanObservationNoise);
		}
	}
}

void DeviceManipulationTabController::setMotionCompensationMovingAverageWindow(unsigned window, bool notify) {
	if (motionCompensationMovingAverageWindow != window) {
		motionCompensationMovingAverageWindow = window;
		parent->vrInputEmulator().setMotionCompensationMovingAverageWindow(motionCompensationMovingAverageWindow);
		saveDeviceManipulationSettings();
		if (notify) {
			emit motionCompensationMovingAverageWindowChanged(motionCompensationMovingAverageWindow);
		}
	}
}

unsigned DeviceManipulationTabController::getDigitalButtonCount(unsigned deviceIndex) {
	unsigned count = 0;
	if (deviceIndex < deviceInfos.size()) {
		vr::ETrackedPropertyError pError;
		auto supportedButtons = vr::VRSystem()->GetUint64TrackedDeviceProperty(deviceInfos[deviceIndex]->openvrId, vr::Prop_SupportedButtons_Uint64, &pError);
		if (pError == vr::TrackedProp_Success) {
			// This is not the most efficient.
			// ToDo: Replace with a proper SWAR popcount algorithm for uint64_t
			for (int i = 0; i < vr::k_EButton_Max; i++) {
				auto buttonMask = vr::ButtonMaskFromId((vr::EVRButtonId)i);
				if (supportedButtons & buttonMask) {
					count++;
				}
			}
		} else {
			LOG(ERROR) << "Could not get supported buttons for device " << deviceInfos[deviceIndex]->serial;
		}
	}
	return count;
}

int DeviceManipulationTabController::getDigitalButtonId(unsigned deviceIndex, unsigned buttonIndex) {
	int buttonId = -1;
	if (deviceIndex < deviceInfos.size()) {
		vr::ETrackedPropertyError pError;
		auto supportedButtons = vr::VRSystem()->GetUint64TrackedDeviceProperty(deviceInfos[deviceIndex]->openvrId, vr::Prop_SupportedButtons_Uint64, &pError);
		if (pError == vr::TrackedProp_Success) {
			// This is not the most efficient.
			// ToDo: Replace with a proper SWAR popcount algorithm for uint64_t
			for (int i = 0; i < vr::k_EButton_Max; i++) {
				auto buttonMask = vr::ButtonMaskFromId((vr::EVRButtonId)i);
				if (supportedButtons & buttonMask) {
					if (buttonIndex <= 0) {
						buttonId = i;
						break;
					} else {
						buttonIndex--;
					}
				}
			}
		} else {
			LOG(ERROR) << "Could not get supported buttons for device " << deviceInfos[deviceIndex]->serial;
		}
	}
	return buttonId;
}

QString DeviceManipulationTabController::getDigitalButtonName(unsigned deviceIndex, unsigned buttonId) {
	if (deviceIndex < deviceInfos.size()) {
		return parent->openvrButtonToString(deviceInfos[deviceIndex]->openvrId, buttonId);
	} else {
		return parent->openvrButtonToString(vr::k_unTrackedDeviceIndexInvalid, buttonId);
	}
}

QString DeviceManipulationTabController::getDigitalButtonStatus(unsigned deviceIndex, unsigned buttonId) {
	QString status;
	if (deviceIndex < deviceInfos.size()) {
		auto remapping = parent->vrInputEmulator().getDigitalInputRemapping(deviceInfos[deviceIndex]->openvrId, buttonId);
		if (!remapping.doublePressEnabled && !remapping.longPressEnabled) {
			status = parent->digitalBindingToString(remapping.binding, remapping.binding.data.openvr.controllerId != deviceInfos[deviceIndex]->openvrId);
		} else {
			status.append("Regular: ");
			if (remapping.binding.type == vrinputemulator::DigitalBindingType::NoRemapping) {
				status.append("Original");
			} else {
				status.append(parent->digitalBindingToString(remapping.binding, remapping.binding.data.openvr.controllerId != deviceInfos[deviceIndex]->openvrId));
			}
			if (remapping.longPressEnabled) {
				status.append("; Long: ");
				if (remapping.longPressBinding.type == vrinputemulator::DigitalBindingType::NoRemapping) {
					status.append("Original");
				} else {
					status.append(parent->digitalBindingToString(remapping.longPressBinding, remapping.longPressBinding.data.openvr.controllerId != deviceInfos[deviceIndex]->openvrId));
				}
			}
			if (remapping.doublePressEnabled) {
				status.append("; Double: ");
				if (remapping.doublePressBinding.type == vrinputemulator::DigitalBindingType::NoRemapping) {
					status.append("Original");
				} else {
					status.append(parent->digitalBindingToString(remapping.doublePressBinding, remapping.doublePressBinding.data.openvr.controllerId != deviceInfos[deviceIndex]->openvrId));
				}
			}
			if (status.size() > 60) {
				status = status.left(60).append("...");
			}
		}
	}
	return status;
}

unsigned DeviceManipulationTabController::getAnalogAxisCount(unsigned deviceIndex) {
	unsigned count = 0;
	if (deviceIndex < deviceInfos.size()) {
		for (int i = 0; i < 5; i++) {
			vr::ETrackedPropertyError pError;
			auto axisType = vr::VRSystem()->GetInt32TrackedDeviceProperty(deviceInfos[deviceIndex]->openvrId, (vr::ETrackedDeviceProperty)((int)vr::Prop_Axis0Type_Int32 + i), &pError);
			if (pError == vr::TrackedProp_Success) {
				if (axisType != vr::k_eControllerAxis_None) {
					count++;
				}
			} else {
				LOG(ERROR) << "Could not get axis types for device " << deviceInfos[deviceIndex]->serial;
				break;
			}
		}
	}
	return count;
}

int DeviceManipulationTabController::getAnalogAxisId(unsigned deviceIndex, unsigned axisIndex) {
	int axisId = -1;
	if (deviceIndex < deviceInfos.size()) {
		for (int i = 0; i < 5; i++) {
			vr::ETrackedPropertyError pError;
			auto axisType = vr::VRSystem()->GetInt32TrackedDeviceProperty(deviceInfos[deviceIndex]->openvrId, (vr::ETrackedDeviceProperty)((int)vr::Prop_Axis0Type_Int32 + i), &pError);
			if (pError == vr::TrackedProp_Success) {
				if (axisType != vr::k_eControllerAxis_None) {
					if (axisIndex <= 0) {
						axisId = i;
						break;
					} else {
						axisIndex--;
					}
				}
			} else {
				LOG(ERROR) << "Could not get axis types for device " << deviceInfos[deviceIndex]->serial;
				break;
			}
		}
	}
	return axisId;
}

QString DeviceManipulationTabController::getAnalogAxisName(unsigned deviceIndex, unsigned axisId) {
	if (deviceIndex < deviceInfos.size()) {
		return parent->openvrAxisToString(deviceInfos[deviceIndex]->openvrId, axisId);
	} else {
		return parent->openvrAxisToString(vr::k_unTrackedDeviceIndexInvalid, axisId);
	}
}

QString DeviceManipulationTabController::getAnalogAxisStatus(unsigned deviceIndex, unsigned axisId) {
	QString status;
	if (deviceIndex < deviceInfos.size()) {
		auto remapping = parent->vrInputEmulator().getAnalogInputRemapping(deviceInfos[deviceIndex]->openvrId, axisId);
		status = parent->analogBindingToString(remapping.binding, remapping.binding.data.openvr.controllerId != deviceInfos[deviceIndex]->openvrId);
		if (status.size() > 60) {
			status = status.left(60).append("...");
		}
	}
	return status;
}

void DeviceManipulationTabController::startConfigureDigitalInputRemapping(unsigned deviceIndex, unsigned buttonId) {
	if (deviceIndex < deviceInfos.size()) {
		auto remapping  = parent->vrInputEmulator().getDigitalInputRemapping(deviceInfos[deviceIndex]->openvrId, buttonId);
		if (!remapping.valid) {
			remapping = vrinputemulator::DigitalInputRemapping(true);
		}
		parent->digitalInputRemappingController.startConfigureRemapping(remapping, deviceIndex, deviceInfos[deviceIndex]->openvrId, buttonId);
	}
}

void DeviceManipulationTabController::finishConfigureDigitalInputRemapping(unsigned deviceIndex, unsigned buttonId, bool touchAsClick,
		bool longPress, int longPressThreshold, bool longPressImmediateRelease, bool doublePress, int doublePressThreshold, bool doublePressImmediateRelease) {
	auto remapping = parent->digitalInputRemappingController.currentRemapping();
	remapping.touchAsClick = touchAsClick;
	remapping.longPressEnabled = longPress;
	remapping.longPressThreshold = longPressThreshold;
	remapping.longPressImmediateRelease = longPressImmediateRelease;
	remapping.doublePressEnabled = doublePress;
	remapping.doublePressThreshold = doublePressThreshold;
	remapping.doublePressImmediateRelease = doublePressImmediateRelease;
	parent->vrInputEmulator().setDigitalInputRemapping(deviceInfos[deviceIndex]->openvrId, buttonId, remapping);
	emit configureDigitalInputRemappingFinished();
}

void DeviceManipulationTabController::startConfigureAnalogInputRemapping(unsigned deviceIndex, unsigned axisId) {
	if (deviceIndex < deviceInfos.size()) {
		auto remapping = parent->vrInputEmulator().getAnalogInputRemapping(deviceInfos[deviceIndex]->openvrId, axisId);
		if (!remapping.valid) {
			remapping = vrinputemulator::AnalogInputRemapping(true);
		}
		parent->analogInputRemappingController.startConfigureRemapping(remapping, deviceIndex, deviceInfos[deviceIndex]->openvrId, axisId);
	}
}

void DeviceManipulationTabController::finishConfigureAnalogInputRemapping(unsigned deviceIndex, unsigned axisId) {
	auto remapping = parent->analogInputRemappingController.currentRemapping();
	parent->vrInputEmulator().setAnalogInputRemapping(deviceInfos[deviceIndex]->openvrId, axisId, remapping);
	emit configureAnalogInputRemappingFinished();
}

unsigned DeviceManipulationTabController::getRenderModelCount() {
	return (unsigned)vr::VRRenderModels()->GetRenderModelCount();
}

QString DeviceManipulationTabController::getRenderModelName(unsigned index) {
	char buffer[vr::k_unMaxPropertyStringSize];
	vr::VRRenderModels()->GetRenderModelName(index, buffer, vr::k_unMaxPropertyStringSize);
	return buffer;
}


void DeviceManipulationTabController::enableDeviceOffsets(unsigned index, bool enable, bool notify) {
	if (index < deviceInfos.size()) {
		try {
			parent->vrInputEmulator().enableDeviceOffsets(deviceInfos[index]->openvrId, enable);
			deviceInfos[index]->deviceOffsetsEnabled = enable;
		} catch (const std::exception& e) {
			LOG(ERROR) << "Exception caught while setting translation offset: " << e.what();
		}
		if (notify) {
			updateDeviceInfo(index);
			emit deviceInfoChanged(index);
		}
	}
}

void DeviceManipulationTabController::setWorldFromDriverRotationOffset(unsigned index, double yaw, double pitch, double roll, bool notify) {
	if (index < deviceInfos.size()) {
		try {
			parent->vrInputEmulator().setWorldFromDriverRotationOffset(deviceInfos[index]->openvrId, vrmath::quaternionFromYawPitchRoll(yaw * 0.01745329252, pitch * 0.01745329252, roll * 0.01745329252));
			deviceInfos[index]->worldFromDriverRotationOffset.v[0] = yaw;
			deviceInfos[index]->worldFromDriverRotationOffset.v[1] = pitch;
			deviceInfos[index]->worldFromDriverRotationOffset.v[2] = roll;
		} catch (const std::exception& e) {
			LOG(ERROR) << "Exception caught while setting WorldFromDriver rotation offset: " << e.what();
		}
		if (notify) {
			updateDeviceInfo(index);
			emit deviceInfoChanged(index);
		}
	}
}

void DeviceManipulationTabController::setWorldFromDriverTranslationOffset(unsigned index, double x, double y, double z, bool notify) {
	if (index < deviceInfos.size()) {
		try {
			parent->vrInputEmulator().setWorldFromDriverTranslationOffset(deviceInfos[index]->openvrId, { x * 0.01, y * 0.01, z * 0.01 });
			deviceInfos[index]->worldFromDriverTranslationOffset.v[0] = x;
			deviceInfos[index]->worldFromDriverTranslationOffset.v[1] = y;
			deviceInfos[index]->worldFromDriverTranslationOffset.v[2] = z;
		} catch (const std::exception& e) {
			LOG(ERROR) << "Exception caught while setting WorldFromDriver translation offset: " << e.what();
		}
		if (notify) {
			updateDeviceInfo(index);
			emit deviceInfoChanged(index);
		}
	}
}

void DeviceManipulationTabController::setDriverFromHeadRotationOffset(unsigned index, double yaw, double pitch, double roll, bool notify) {
	if (index < deviceInfos.size()) {
		try {
			parent->vrInputEmulator().setDriverFromHeadRotationOffset(deviceInfos[index]->openvrId, vrmath::quaternionFromYawPitchRoll(yaw * 0.01745329252, pitch * 0.01745329252, roll * 0.01745329252));
			deviceInfos[index]->driverFromHeadRotationOffset.v[0] = yaw;
			deviceInfos[index]->driverFromHeadRotationOffset.v[1] = pitch;
			deviceInfos[index]->driverFromHeadRotationOffset.v[2] = roll;
		} catch (const std::exception& e) {
			LOG(ERROR) << "Exception caught while setting DriverFromHead rotation offset: " << e.what();
		}
		if (notify) {
			updateDeviceInfo(index);
			emit deviceInfoChanged(index);
		}
	}
}

void DeviceManipulationTabController::setDriverFromHeadTranslationOffset(unsigned index, double x, double y, double z, bool notify) {
	if (index < deviceInfos.size()) {
		try {
			parent->vrInputEmulator().setDriverFromHeadTranslationOffset(deviceInfos[index]->openvrId, { x * 0.01, y * 0.01, z * 0.01 });
			deviceInfos[index]->driverFromHeadTranslationOffset.v[0] = x;
			deviceInfos[index]->driverFromHeadTranslationOffset.v[1] = y;
			deviceInfos[index]->driverFromHeadTranslationOffset.v[2] = z;
		} catch (const std::exception& e) {
			LOG(ERROR) << "Exception caught while setting WorldFromDriver translation offset: " << e.what();
		}
		if (notify) {
			updateDeviceInfo(index);
			emit deviceInfoChanged(index);
		}
	}
}

void DeviceManipulationTabController::setDriverRotationOffset(unsigned index, double yaw, double pitch, double roll, bool notify) {
	if (index < deviceInfos.size()) {
		try {
			parent->vrInputEmulator().setDriverRotationOffset(deviceInfos[index]->openvrId, vrmath::quaternionFromYawPitchRoll(yaw * 0.01745329252, pitch * 0.01745329252, roll * 0.01745329252));
			deviceInfos[index]->deviceRotationOffset.v[0] = yaw;
			deviceInfos[index]->deviceRotationOffset.v[1] = pitch;
			deviceInfos[index]->deviceRotationOffset.v[2] = roll;
		} catch (const std::exception& e) {
			LOG(ERROR) << "Exception caught while setting Driver rotation offset: " << e.what();
		}
		if (notify) {
			updateDeviceInfo(index);
			emit deviceInfoChanged(index);
		}
	}
}

void DeviceManipulationTabController::setDriverTranslationOffset(unsigned index, double x, double y, double z, bool notify) {
	if (index < deviceInfos.size()) {
		try {
			parent->vrInputEmulator().setDriverTranslationOffset(deviceInfos[index]->openvrId, { x * 0.01, y * 0.01, z * 0.01 });
			deviceInfos[index]->deviceTranslationOffset.v[0] = x;
			deviceInfos[index]->deviceTranslationOffset.v[1] = y;
			deviceInfos[index]->deviceTranslationOffset.v[2] = z;
		} catch (const std::exception& e) {
			LOG(ERROR) << "Exception caught while setting WorldFromDriver translation offset: " << e.what();
		}
		if (notify) {
			updateDeviceInfo(index);
			emit deviceInfoChanged(index);
		}
	}
}

// 0 .. normal, 1 .. disable, 2 .. redirect mode, 3 .. swap mode, 4 ... motion compensation
bool DeviceManipulationTabController::setDeviceMode(unsigned index, unsigned mode, unsigned targedIndex, bool notify) {
	bool retval = true;
	try {
		switch (mode) {
		case 0:
			parent->vrInputEmulator().setDeviceNormalMode(deviceInfos[index]->openvrId);
			break;
		case 1:
			parent->vrInputEmulator().setDeviceFakeDisconnectedMode(deviceInfos[index]->openvrId);
			break;
		case 2:
			parent->vrInputEmulator().setDeviceRedictMode(deviceInfos[index]->openvrId, deviceInfos[targedIndex]->openvrId);
			break;
		case 3:
			parent->vrInputEmulator().setDeviceSwapMode(deviceInfos[index]->openvrId, deviceInfos[targedIndex]->openvrId);
			break;
		case 4:
			if (motionCompensationVelAccMode == vrinputemulator::MotionCompensationVelAccMode::KalmanFilter) {
				parent->vrInputEmulator().setMotionCompensationKalmanProcessNoise(motionCompensationKalmanProcessNoise);
				parent->vrInputEmulator().setMotionCompensationKalmanObservationNoise(motionCompensationKalmanObservationNoise);
			} else if (motionCompensationVelAccMode == vrinputemulator::MotionCompensationVelAccMode::LinearApproximation) {
				parent->vrInputEmulator().setMotionCompensationMovingAverageWindow(motionCompensationMovingAverageWindow);
			}
			parent->vrInputEmulator().setDeviceMotionCompensationMode(deviceInfos[index]->openvrId, motionCompensationVelAccMode);
			break;
		default:
			retval = false;
			m_deviceModeErrorString = "Unknown Device Mode";
			LOG(ERROR) << "Unkown device mode";
			break;
		}
	} catch (vrinputemulator::vrinputemulator_exception& e) {
		retval = false;
		switch (e.errorcode) {
			case (int)vrinputemulator::ipc::ReplyStatus::Ok: {
				m_deviceModeErrorString = "Not an error";
			} break;
			case (int)vrinputemulator::ipc::ReplyStatus::AlreadyInUse: {
				m_deviceModeErrorString = "Device already in use";
			} break;
			case (int)vrinputemulator::ipc::ReplyStatus::InvalidId: {
				m_deviceModeErrorString = "Invalid Id";
			} break;
			case (int)vrinputemulator::ipc::ReplyStatus::NotFound: {
				m_deviceModeErrorString = "Device not found";
			} break;
			case (int)vrinputemulator::ipc::ReplyStatus::NotTracking: {
				m_deviceModeErrorString = "Device not tracking";
			} break;
			default: {
				m_deviceModeErrorString = "Unknown error";
			} break;
		}
		LOG(ERROR) << "Exception caught while setting device mode: " << e.what();
	} catch (std::exception& e) {
		retval = false;
		m_deviceModeErrorString = "Unknown exception";
		LOG(ERROR) << "Exception caught while setting device mode: " << e.what();
	}
	if (notify) {
		updateDeviceInfo(index);
		emit deviceInfoChanged(index);
	}
	return retval;
}

QString DeviceManipulationTabController::getDeviceModeErrorString() {
	return m_deviceModeErrorString;
}


bool DeviceManipulationTabController::updateDeviceInfo(unsigned index) {
	bool retval = false;
	if (index < deviceInfos.size()) {
		try {
			vrinputemulator::DeviceInfo info;
			parent->vrInputEmulator().getDeviceInfo(deviceInfos[index]->openvrId, info);
			if (deviceInfos[index]->deviceMode != info.deviceMode) {
				deviceInfos[index]->deviceMode = info.deviceMode;
				retval = true;
			}
			if (deviceInfos[index]->refDeviceId != info.refDeviceId) {
				deviceInfos[index]->refDeviceId = info.refDeviceId;
				retval = true;
			}
			if (deviceInfos[index]->deviceOffsetsEnabled != info.offsetsEnabled) {
				deviceInfos[index]->deviceOffsetsEnabled = info.offsetsEnabled;
				retval = true;
			}
			if (deviceInfos[index]->deviceMode == 2 || deviceInfos[index]->deviceMode == 3) {
				auto status = info.redirectSuspended ? 1 : 0;
				if (deviceInfos[index]->deviceStatus != status) {
					deviceInfos[index]->deviceStatus = status;
					retval = true;
				}
			}
		} catch (std::exception& e) {
			LOG(ERROR) << "Exception caught while getting device info: " << e.what();
		}
	}
	return retval;
}

void DeviceManipulationTabController::triggerHapticPulse(unsigned index) {
	try {
		// When I use a thread everything works in debug modus, but as soon as I switch to release mode I get a segmentation fault
		// Hard to debug since it works in debug modus and therefore I cannot use a debugger :-(
		for (unsigned i = 0; i < 100; ++i) {
			parent->vrInputEmulator().triggerHapticPulse(deviceInfos[index]->openvrId, 0, 2000, true);
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		/*if (identifyThread.joinable()) {
			identifyThread.join();
		}
		identifyThread = std::thread([](uint32_t openvrId, vrinputemulator::VRInputEmulator* vrInputEmulator) {
			for (unsigned i = 0; i < 50; ++i) {
				vrInputEmulator->triggerHapticPulse(openvrId, 0, 2000, true);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}, this->deviceInfos[index]->openvrId, &vrInputEmulator);*/
	} catch (std::exception& e) {
		LOG(ERROR) << "Exception caught while triggering haptic pulse: " << e.what();
	}
}

void DeviceManipulationTabController::setDeviceRenderModel(unsigned deviceIndex, unsigned renderModelIndex) {
	if (deviceIndex < deviceInfos.size()) {
		if (renderModelIndex == 0) {
			if (deviceInfos[deviceIndex]->renderModelOverlay != vr::k_ulOverlayHandleInvalid) {
				vr::VROverlay()->DestroyOverlay(deviceInfos[deviceIndex]->renderModelOverlay);
				deviceInfos[deviceIndex]->renderModelOverlay = vr::k_ulOverlayHandleInvalid;
			}
		} else {
			vr::VROverlayHandle_t overlayHandle = deviceInfos[deviceIndex]->renderModelOverlay;
			if (overlayHandle == vr::k_ulOverlayHandleInvalid) {
				std::string overlayName = std::string("RenderModelOverlay_") + std::string(deviceInfos[deviceIndex]->serial);
				auto oerror = vr::VROverlay()->CreateOverlay(overlayName.c_str(), overlayName.c_str(), &overlayHandle);
				if (oerror == vr::VROverlayError_None) {
					overlayHandle = deviceInfos[deviceIndex]->renderModelOverlay = overlayHandle;
				} else {
					LOG(ERROR) << "Could not create render model overlay: " << vr::VROverlay()->GetOverlayErrorNameFromEnum(oerror);
				}
			}
			if (overlayHandle != vr::k_ulOverlayHandleInvalid) {
				std::string texturePath = QApplication::applicationDirPath().toStdString() + "\\res\\transparent.png";
				if (QFile::exists(QString::fromStdString(texturePath))) {
					vr::VROverlay()->SetOverlayFromFile(overlayHandle, texturePath.c_str());
					char buffer[vr::k_unMaxPropertyStringSize];
					vr::VRRenderModels()->GetRenderModelName(renderModelIndex - 1, buffer, vr::k_unMaxPropertyStringSize);
					vr::VROverlay()->SetOverlayRenderModel(overlayHandle, buffer, nullptr);
					vr::HmdMatrix34_t trans = {
						1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f
					};
					vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(overlayHandle, deviceInfos[deviceIndex]->openvrId, &trans);
					vr::VROverlay()->ShowOverlay(overlayHandle);
				} else {
					LOG(ERROR) << "Could not find texture \"" << texturePath << "\"";
				}
			}
		}
	}
}

} // namespace inputemulator
