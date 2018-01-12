
#include "ServerDriver.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "VirtualDeviceDriver.h"
#include "../hooks/common.h"
#include "../devicemanipulation/DeviceManipulationHandle.h"


namespace vrinputemulator {
namespace driver {



ServerDriver* ServerDriver::singleton = nullptr;
std::string ServerDriver::installDir;


ServerDriver::ServerDriver() : m_motionCompensation(this) {
	singleton = this;
	memset(m_openvrIdToVirtualDeviceMap, 0, sizeof(VirtualDeviceDriver*) * vr::k_unMaxTrackedDeviceCount);
	memset(_openvrIdToDeviceManipulationHandleMap, 0, sizeof(DeviceManipulationHandle*) * vr::k_unMaxTrackedDeviceCount);
}


ServerDriver::~ServerDriver() {
	LOG(TRACE) << "CServerDriver::~CServerDriver_InputEmulator()";
}


bool ServerDriver::hooksTrackedDevicePoseUpdated(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::DriverPose_t& newPose, uint32_t& unPoseStructSize) {
	if (_openvrIdToDeviceManipulationHandleMap[unWhichDevice] && _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->isValid()) {
		return _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->handlePoseUpdate(serverDriverHost, version, unWhichDevice, newPose, unPoseStructSize);
	}
	return true;
}


bool ServerDriver::hooksTrackedDeviceButtonPressed(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::EVRButtonId& eButtonId, double& eventTimeOffset) {
	LOG(TRACE) << "ServerDriver::hooksTrackedDeviceButtonPressed(" << serverDriverHost << ", " << version << ", " << unWhichDevice << ", " << (int)eButtonId << ", " << eventTimeOffset << ")";
	if (_openvrIdToDeviceManipulationHandleMap[unWhichDevice] && _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->isValid()) {
		return _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->handleButtonEvent(serverDriverHost, version, unWhichDevice, ButtonEventType::ButtonPressed, eButtonId, eventTimeOffset);
	}
	return true;
}

bool ServerDriver::hooksTrackedDeviceButtonUnpressed(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::EVRButtonId& eButtonId, double& eventTimeOffset) {
	LOG(TRACE) << "ServerDriver::hooksTrackedDeviceButtonUnpressed(" << serverDriverHost << ", " << version << ", " << unWhichDevice << ", " << (int)eButtonId << ", " << eventTimeOffset << ")";
	if (_openvrIdToDeviceManipulationHandleMap[unWhichDevice] && _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->isValid()) {
		return _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->handleButtonEvent(serverDriverHost, version, unWhichDevice, ButtonEventType::ButtonUnpressed, eButtonId, eventTimeOffset);
	}
	return true;
}

bool ServerDriver::hooksTrackedDeviceButtonTouched(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::EVRButtonId& eButtonId, double& eventTimeOffset) {
	LOG(TRACE) << "ServerDriver::hooksTrackedDeviceButtonTouched(" << serverDriverHost << ", " << version << ", " << unWhichDevice << ", " << (int)eButtonId << ", " << eventTimeOffset << ")";
	if (_openvrIdToDeviceManipulationHandleMap[unWhichDevice] && _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->isValid()) {
		return _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->handleButtonEvent(serverDriverHost, version, unWhichDevice, ButtonEventType::ButtonTouched, eButtonId, eventTimeOffset);
	}
	return true;
}

bool ServerDriver::hooksTrackedDeviceButtonUntouched(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::EVRButtonId& eButtonId, double& eventTimeOffset) {
	LOG(TRACE) << "ServerDriver::hooksTrackedDeviceButtonUntouched(" << serverDriverHost << ", " << version << ", " << unWhichDevice << ", " << (int)eButtonId << ", " << eventTimeOffset << ")";
	if (_openvrIdToDeviceManipulationHandleMap[unWhichDevice] && _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->isValid()) {
		return _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->handleButtonEvent(serverDriverHost, version, unWhichDevice, ButtonEventType::ButtonUntouched, eButtonId, eventTimeOffset);
	}
	return true;
}

bool ServerDriver::hooksTrackedDeviceAxisUpdated(void* serverDriverHost, int version, uint32_t& unWhichDevice, uint32_t& unWhichAxis, vr::VRControllerAxis_t& axisState) {
	LOG(TRACE) << "ServerDriver::hooksTrackedDeviceAxisUpdated(" << serverDriverHost << ", " << version << ", " << unWhichDevice << ", " << (int)unWhichAxis << ")";
	if (_openvrIdToDeviceManipulationHandleMap[unWhichDevice] && _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->isValid()) {
		return _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->handleAxisUpdate(serverDriverHost, version, unWhichDevice, unWhichAxis, axisState);
	}
	return true;
}


bool ServerDriver::hooksControllerTriggerHapticPulse(void* controllerComponent, int version, uint32_t& unAxisId, uint16_t& usPulseDurationMicroseconds) {
	LOG(TRACE) << "ServerDriver::hooksControllerTriggerHapticPulse(" << controllerComponent << ", " << version << ", " << unAxisId << ", " << usPulseDurationMicroseconds << ")";
	auto it = _ptrToDeviceManipulationHandleMap.find(controllerComponent);
	if (it != _ptrToDeviceManipulationHandleMap.end()) {
		it->second->triggerHapticPulse(unAxisId, usPulseDurationMicroseconds);
		return false;
	}
	return true;
}

void ServerDriver::hooksTrackedDeviceAdded(void* serverDriverHost, int version, const char *pchDeviceSerialNumber, vr::ETrackedDeviceClass& eDeviceClass, void* pDriver) {
	LOG(TRACE) << "ServerDriver::hooksTrackedDeviceAdded(" << serverDriverHost << ", " << version << ", " << pchDeviceSerialNumber << ", " << (int)eDeviceClass << ", " << pDriver << ")";

	// Device Class Override
	if (eDeviceClass == vr::TrackedDeviceClass_GenericTracker && _propertiesOverrideGenericTrackerFakeController) {
		eDeviceClass = vr::TrackedDeviceClass_Controller;
		LOG(INFO) << "Disguised GenericTracker " << pchDeviceSerialNumber << " as Controller.";
	}

	// Create ManipulationInfo entry
	auto handle = std::make_shared<DeviceManipulationHandle>(eDeviceClass, pDriver, serverDriverHost, version);
	_deviceManipulationHandles.insert({pDriver, handle});

	// Hook into server driver interface
	handle->setServerDriverHooks(InterfaceHooks::hookInterface(pDriver, "ITrackedDeviceServerDriver_005"));

	// Hook into controller component interface if available
	auto controllerComponent = (vr::IVRControllerComponent*)((vr::ITrackedDeviceServerDriver*)pDriver)->GetComponent(vr::IVRControllerComponent_Version);
	if (controllerComponent) {
		handle->setControllerComponentHooks(InterfaceHooks::hookInterface(controllerComponent, "IVRControllerComponent_001"));
		_ptrToDeviceManipulationHandleMap[controllerComponent] = handle.get();
	}
}


void ServerDriver::hooksTrackedDeviceActivated(void* serverDriver, int version, uint32_t unObjectId) {
	LOG(TRACE) << "ServerDriver::hooksTrackedDeviceActivated(" << serverDriver << ", " << version << ", " << unObjectId << ")";
	auto i = _deviceManipulationHandles.find(serverDriver);
	if (i != _deviceManipulationHandles.end()) {
		auto handle = i->second;
		handle->setOpenvrId(unObjectId);
		_openvrIdToDeviceManipulationHandleMap[unObjectId] = handle.get();

		// get device property container
		auto container = vr::VRPropertiesRaw()->TrackedDeviceToPropertyContainer(unObjectId);
		handle->setPropertyContainer(container);
		_propertyContainerToDeviceManipulationHandleMap[container] = handle.get();

		LOG(INFO) << "ServerDriver::hooksTrackedDeviceActivated: Successfully created DeviceManipulationHandle for device #" << unObjectId;
	}
}


std::string _propertyValueToString(void *pvBuffer, uint32_t unBufferSize, vr::PropertyTypeTag_t unTag) {
	switch (unTag) {
	case vr::k_unFloatPropertyTag:
		return std::to_string(*(float*)pvBuffer) + " [float]";
		break;
	case vr::k_unInt32PropertyTag:
		return std::to_string(*(int32_t*)pvBuffer) + " [int32]";
		break;
	case vr::k_unUint64PropertyTag:
		return std::to_string(*(uint64_t*)pvBuffer) + " [uint64]";
		break;
	case vr::k_unBoolPropertyTag:
		return std::to_string(*(bool*)pvBuffer) + " [bool]";
		break;
	case vr::k_unStringPropertyTag:
		return std::string((const char*)pvBuffer) + " [string]";
		break;
	case vr::k_unHmdMatrix34PropertyTag:
		return std::string("[matrix34]");
		break;
	case vr::k_unHmdMatrix44PropertyTag:
		return std::string("[matrix44]");
		break;
	case vr::k_unHmdVector3PropertyTag:
		return std::string("[vector3]");
		break;
	case vr::k_unHmdVector4PropertyTag:
		return std::string("[vector4]");
		break;
	case vr::k_unHiddenAreaPropertyTag:
		return std::string("[HiddenAreaProperty]");
		break;
	case vr::k_unInvalidPropertyTag:
		return std::string("[Invalid]");
		break;
	default:
		return std::string("<Unknown>");
		break;
	}
}

void ServerDriver::hooksPropertiesReadPropertyBatch(void* properties, vr::PropertyContainerHandle_t ulContainer, void* pBatch, uint32_t unBatchEntryCount) {
}


void ServerDriver::hooksPropertiesWritePropertyBatch(void* properties, vr::PropertyContainerHandle_t ulContainer, void* pBatch, uint32_t unBatchEntryCount) {
	LOG(TRACE) << "ServerDriver::hooksPropertiesWritePropertyBatch(" << properties << ", " << (uint64_t)ulContainer << ", " << (void*)pBatch << ", " << unBatchEntryCount << ")";
	uint32_t deviceId = vr::k_unTrackedDeviceIndexInvalid;
	DeviceManipulationHandle* deviceHandle = nullptr;
	auto entryIt = _propertyContainerToDeviceManipulationHandleMap.find(ulContainer);
	if (entryIt != _propertyContainerToDeviceManipulationHandleMap.end()) {
		deviceHandle = entryIt->second;
		deviceId = deviceHandle->openvrId();
	} else {
		for (uint32_t id = 0; id < vr::k_unMaxTrackedDeviceCount; id++) {
			auto propsContainer = vr::VRPropertiesRaw()->TrackedDeviceToPropertyContainer(id);
			if (propsContainer == ulContainer) {
				deviceId = id;
				break;
			}
		}
	}
	for (uint32_t i = 0; i < unBatchEntryCount; i++) {
		vr::PropertyWrite_t& be = ((vr::PropertyWrite_t*)pBatch)[i];
		LOG(TRACE) << "\tProperty "<< i << ": " << (int)be.prop << " = " << _propertyValueToString(be.pvBuffer, be.unBufferSize, be.unTag);
		if (be.prop == vr::Prop_ManufacturerName_String && !_propertiesOverrideHmdManufacturer.empty()) {
			LOG(INFO) << "Overwriting Device Manufacturer: " << be.pvBuffer << " => " << _propertiesOverrideHmdManufacturer;
			be.pvBuffer = (void*)_propertiesOverrideHmdManufacturer.c_str();
			be.unBufferSize = (uint32_t)_propertiesOverrideHmdManufacturer.size() + 1;
		} else if (deviceId == vr::k_unTrackedDeviceIndex_Hmd && be.prop == vr::Prop_ModelNumber_String && !_propertiesOverrideHmdModel.empty()) {
			LOG(INFO) << "Overwriting Hmd Model: " << be.pvBuffer << " => " << _propertiesOverrideHmdModel;
			be.pvBuffer = (void*)_propertiesOverrideHmdModel.c_str();
			be.unBufferSize = (uint32_t)_propertiesOverrideHmdModel.size() + 1;
		} else if (be.prop == vr::Prop_TrackingSystemName_String && !_propertiesOverrideHmdTrackingSystem.empty()) {
			LOG(INFO) << "Overwriting Device TrackingSystem: " << be.pvBuffer << " => " << _propertiesOverrideHmdTrackingSystem;
			be.pvBuffer = (void*)_propertiesOverrideHmdTrackingSystem.c_str();
			be.unBufferSize = (uint32_t)_propertiesOverrideHmdTrackingSystem.size() + 1;
		}
	}
}


vr::EVRInitError ServerDriver::Init(vr::IVRDriverContext *pDriverContext) {
	LOG(TRACE) << "CServerDriver::Init()";

	// Initialize Hooking
	InterfaceHooks::setServerDriver(this);
	auto mhError = MH_Initialize();
	if (mhError == MH_OK) {
		_driverContextHooks = InterfaceHooks::hookInterface(pDriverContext, "IVRDriverContext");
	} else {
		LOG(ERROR) << "Error while initialising minHook: " << MH_StatusToString(mhError);
	}

	LOG(DEBUG) << "Initialize driver context.";
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

	// Read installation directory
	vr::ETrackedPropertyError tpeError;
	installDir = vr::VRProperties()->GetStringProperty(pDriverContext->GetDriverHandle(), vr::Prop_InstallPath_String, &tpeError);
	if (tpeError == vr::TrackedProp_Success) {
		LOG(INFO) << "Install Dir:" << installDir;
	} else {
		LOG(INFO) << "Could not get Install Dir: " << vr::VRPropertiesRaw()->GetPropErrorNameFromEnum(tpeError);
	}

	// Read vrsettings
	char buffer[vr::k_unMaxPropertyStringSize];
	vr::EVRSettingsError peError;
	vr::VRSettings()->GetString(vrsettings_SectionName, vrsettings_overrideHmdManufacturer_string, buffer, vr::k_unMaxPropertyStringSize, &peError);
	if (peError == vr::VRSettingsError_None) {
		_propertiesOverrideHmdManufacturer = buffer;
		LOG(INFO) << vrsettings_SectionName << "::" << vrsettings_overrideHmdManufacturer_string << " = " << _propertiesOverrideHmdManufacturer;
	}
	vr::VRSettings()->GetString(vrsettings_SectionName, vrsettings_overrideHmdModel_string, buffer, vr::k_unMaxPropertyStringSize, &peError);
	if (peError == vr::VRSettingsError_None) {
		_propertiesOverrideHmdModel = buffer;
		LOG(INFO) << vrsettings_SectionName << "::" << vrsettings_overrideHmdModel_string << " = " << _propertiesOverrideHmdModel;
	}
	vr::VRSettings()->GetString(vrsettings_SectionName, vrsettings_overrideHmdTrackingSystem_string, buffer, vr::k_unMaxPropertyStringSize, &peError);
	if (peError == vr::VRSettingsError_None) {
		_propertiesOverrideHmdTrackingSystem = buffer;
		LOG(INFO) << vrsettings_SectionName << "::" << vrsettings_overrideHmdTrackingSystem_string << " = " << _propertiesOverrideHmdTrackingSystem;
	}
	auto boolVal = vr::VRSettings()->GetBool(vrsettings_SectionName, vrsettings_genericTrackerFakeController_bool, &peError);
	if (peError == vr::VRSettingsError_None) {
		_propertiesOverrideGenericTrackerFakeController = boolVal;
		LOG(INFO) << vrsettings_SectionName << "::" << vrsettings_genericTrackerFakeController_bool << " = " << boolVal;
	}

	// Start IPC thread
	shmCommunicator.init(this);
	return vr::VRInitError_None;
}


void ServerDriver::Cleanup() {
	LOG(TRACE) << "CServerDriver::Cleanup()";
	_driverContextHooks.reset();
	MH_Uninitialize();
	shmCommunicator.shutdown();
	VR_CLEANUP_SERVER_DRIVER_CONTEXT();
}


// Call frequency: ~93Hz
void ServerDriver::RunFrame() {
	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
		auto vd = m_virtualDevices[i];
		if (vd && vd->published() && vd->periodicPoseUpdates()) {
			vd->sendPoseUpdate();
		}
	}
	for (auto d : _deviceManipulationHandles) {
		d.second->RunFrame();
	}
	m_motionCompensation.runFrame();
}


int32_t ServerDriver::virtualDevices_addDevice(VirtualDeviceType type, const std::string& serial) {
	LOG(TRACE) << "CServerDriver::addTrackedDevice( " << serial << " )";
	std::lock_guard<std::recursive_mutex> lock(_virtualDevicesMutex);
	if (m_virtualDeviceCount >= vr::k_unMaxTrackedDeviceCount) {
		return -1;
	}
	for (uint32_t i = 0; i < m_virtualDeviceCount; ++i) {
		if (m_virtualDevices[i]->serialNumber().compare(serial) == 0) {
			return -2;
		}
	}
	uint32_t virtualDeviceId = 0;
	for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
		if (!m_virtualDevices[i]) {
			virtualDeviceId = i;
			break;
		}
	}
	switch (type) {
		case VirtualDeviceType::TrackedController: {
			m_virtualDevices[virtualDeviceId] = std::make_shared<VirtualDeviceDriver>(this, VirtualDeviceType::TrackedController, serial, virtualDeviceId);
			LOG(INFO) << "Added new tracked controller:  type " << (int)type << ", serial \"" << serial << "\", emulatedDeviceId " << virtualDeviceId;
			m_virtualDeviceCount++;
			return virtualDeviceId;
		}
		default:
			return -3;
	}
}


int32_t ServerDriver::virtualDevices_publishDevice(uint32_t emulatedDeviceId, bool notify) {
	LOG(TRACE) << "CServerDriver::publishTrackedDevice( " << emulatedDeviceId << " )";
	std::lock_guard<std::recursive_mutex> lock(_virtualDevicesMutex);
	if (emulatedDeviceId >= m_virtualDeviceCount) {
		return -1;
	} else if (!m_virtualDevices[emulatedDeviceId]) {
		return -2;
	} else {
		auto device = m_virtualDevices[emulatedDeviceId].get();
		if (device->published()) {
			return -3;
		}
		try {
			device->publish();
			LOG(INFO) << "Published tracked controller: virtualDeviceId " << emulatedDeviceId;
		} catch (std::exception& e) {
			LOG(ERROR) << "Error while publishing controller " << emulatedDeviceId << ": " << e.what();
			return -4;
		}
		return 0;
	}
}

void ServerDriver::_trackedDeviceActivated(uint32_t deviceId, VirtualDeviceDriver * device) {
	m_openvrIdToVirtualDeviceMap[deviceId] = device;
}

void ServerDriver::_trackedDeviceDeactivated(uint32_t deviceId) {
	m_openvrIdToVirtualDeviceMap[deviceId] = nullptr;
}

void ServerDriver::openvr_buttonEvent(uint32_t unWhichDevice, ButtonEventType eventType, vr::EVRButtonId eButtonId, double eventTimeOffset) {
	auto devicePtr = this->m_openvrIdToVirtualDeviceMap[unWhichDevice];
	if (devicePtr && devicePtr->deviceType() == VirtualDeviceType::TrackedController) {
		devicePtr->buttonEvent(eventType, eButtonId, eventTimeOffset);
	} else {
		if (_openvrIdToDeviceManipulationHandleMap[unWhichDevice] && _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->isValid()) {
			_openvrIdToDeviceManipulationHandleMap[unWhichDevice]->ll_sendButtonEvent(eventType, eButtonId, eventTimeOffset);
		}
	}
}

void ServerDriver::openvr_axisEvent(uint32_t unWhichDevice, uint32_t unWhichAxis, const vr::VRControllerAxis_t & axisState) {
	auto devicePtr = this->m_openvrIdToVirtualDeviceMap[unWhichDevice];
	if (devicePtr && devicePtr->deviceType() == VirtualDeviceType::TrackedController) {
		devicePtr->axisEvent(unWhichAxis, axisState);
	} else {
		if (_openvrIdToDeviceManipulationHandleMap[unWhichDevice] && _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->isValid()) {
			_openvrIdToDeviceManipulationHandleMap[unWhichDevice]->ll_sendAxisEvent(unWhichAxis, axisState);
		}
	}
}

void ServerDriver::openvr_poseUpdate(uint32_t unWhichDevice, vr::DriverPose_t & newPose, int64_t timestamp) {
	auto devicePtr = this->m_openvrIdToVirtualDeviceMap[unWhichDevice];
	auto now = std::chrono::duration_cast <std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	auto diff = 0.0;
	if (timestamp < now) {
		diff = ((double)now - timestamp) / 1000.0;
	}
	if (devicePtr) {
		devicePtr->updatePose(newPose, -diff);
	} else {
		if (_openvrIdToDeviceManipulationHandleMap[unWhichDevice] && _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->isValid()) {
			newPose.poseTimeOffset -= diff;
			_openvrIdToDeviceManipulationHandleMap[unWhichDevice]->ll_sendPoseUpdate(newPose);
		}
	}
}

void ServerDriver::openvr_proximityEvent(uint32_t unWhichDevice, bool bProximitySensorTriggered) {
	vr::VRServerDriverHost()->ProximitySensorState(unWhichDevice, bProximitySensorTriggered);
}

void ServerDriver::openvr_vendorSpecificEvent(uint32_t unWhichDevice, vr::EVREventType eventType, vr::VREvent_Data_t & eventData, double eventTimeOffset) {
	vr::VRServerDriverHost()->VendorSpecificEvent(unWhichDevice, eventType, eventData, eventTimeOffset);
}

uint32_t ServerDriver::virtualDevices_getDeviceCount() {
	std::lock_guard<std::recursive_mutex> lock(this->_virtualDevicesMutex);
	return this->m_virtualDeviceCount;
}

VirtualDeviceDriver* ServerDriver::virtualDevices_getDevice(uint32_t unWhichDevice) {
	std::lock_guard<std::recursive_mutex> lock(this->_virtualDevicesMutex);
	if (this->m_virtualDevices[unWhichDevice]) {
		return this->m_virtualDevices[unWhichDevice].get();
	}
	return nullptr;
}

VirtualDeviceDriver* ServerDriver::virtualDevices_findDevice(const std::string& serial) {
	for (uint32_t i = 0; i < this->m_virtualDeviceCount; ++i) {
		if (this->m_virtualDevices[i]->serialNumber().compare(serial) == 0) {
			return this->m_virtualDevices[i].get();
		}
	}
	return nullptr;
}

DeviceManipulationHandle* ServerDriver::getDeviceManipulationHandleById(uint32_t unWhichDevice) {
	std::lock_guard<std::recursive_mutex> lock(_deviceManipulationHandlesMutex);
	if (_openvrIdToDeviceManipulationHandleMap[unWhichDevice] && _openvrIdToDeviceManipulationHandleMap[unWhichDevice]->isValid()) {
		return _openvrIdToDeviceManipulationHandleMap[unWhichDevice];
	}
	return nullptr;
}


DeviceManipulationHandle* ServerDriver::getDeviceManipulationHandleByPropertyContainer(vr::PropertyContainerHandle_t container) {
	std::lock_guard<std::recursive_mutex> lock(_deviceManipulationHandlesMutex);
	auto it = _propertyContainerToDeviceManipulationHandleMap.find(container);
	if (it != _propertyContainerToDeviceManipulationHandleMap.end()) {
		return it->second;
	}
	return nullptr;
}

/*void ServerDriver::enableMotionCompensation(bool enable) {
	_motionCompensationZeroRefTimeout = 0;
	_motionCompensationZeroPoseValid = false;
	_motionCompensationRefPoseValid = false;
	_motionCompensationEnabled = enable;
	if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::KalmanFilter) {
		for (auto d : _deviceManipulationHandles) {
			d.second->setLastPoseTime(-1);
		}
	}
}

void ServerDriver::setMotionCompensationRefDevice(DeviceManipulationHandle* device) {
	_motionCompensationRefDevice = device;
}

DeviceManipulationHandle* ServerDriver::getMotionCompensationRefDevice() {
	return _motionCompensationRefDevice;
}

void ServerDriver::setMotionCompensationVelAccMode(MotionCompensationVelAccMode velAccMode) {
	if (_motionCompensationVelAccMode != velAccMode) {
		_motionCompensationRefVelAccValid = false;
		for (auto d : _deviceManipulationHandles) {
			d.second->setLastPoseTime(-1);
			d.second->kalmanFilter().setProcessNoise(m_motionCompensationKalmanProcessVariance);
			d.second->kalmanFilter().setObservationNoise(m_motionCompensationKalmanObservationVariance);
			d.second->velMovingAverage().resize(m_motionCompensationMovingAverageWindow);
		}
		_motionCompensationVelAccMode = velAccMode;
	}
}

void ServerDriver::setMotionCompensationKalmanProcessVariance(double variance) {
	m_motionCompensationKalmanProcessVariance = variance;
	if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::KalmanFilter) {
		for (auto d : _deviceManipulationHandles) {
			d.second->kalmanFilter().setProcessNoise(variance);
		}
	}
}

void ServerDriver::setMotionCompensationKalmanObservationVariance(double variance) {
	m_motionCompensationKalmanObservationVariance = variance;
	if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::KalmanFilter) {
		for (auto d : _deviceManipulationHandles) {
			d.second->kalmanFilter().setObservationNoise(variance);
		}
	}
}

void ServerDriver::setMotionCompensationMovingAverageWindow(unsigned window) {
	m_motionCompensationMovingAverageWindow = window;
	if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::LinearApproximation) {
		for (auto d : _deviceManipulationHandles) {
			d.second->velMovingAverage().resize(window);
		}
	}
}

void ServerDriver::_disableMotionCompensationOnAllDevices() {
	for (auto d : _deviceManipulationHandles) {
		if (d.second->deviceMode() == 5) {
			d.second->setDefaultMode();
		}
	}
}

bool ServerDriver::_isMotionCompensationZeroPoseValid() {
	return _motionCompensationZeroPoseValid;
}

void ServerDriver::_setMotionCompensationZeroPose(const vr::DriverPose_t& pose) {
	// convert pose from driver space to app space
	auto tmpConj = vrmath::quaternionConjugate(pose.qWorldFromDriverRotation);
	_motionCompensationZeroPos = vrmath::quaternionRotateVector(pose.qWorldFromDriverRotation, tmpConj, pose.vecPosition, true) - pose.vecWorldFromDriverTranslation;
	_motionCompensationZeroRot = tmpConj * pose.qRotation;

	_motionCompensationZeroPoseValid = true;
}

void ServerDriver::_updateMotionCompensationRefPose(const vr::DriverPose_t& pose) {
	// convert pose from driver space to app space
	auto tmpConj = vrmath::quaternionConjugate(pose.qWorldFromDriverRotation);
	_motionCompensationRefPos = vrmath::quaternionRotateVector(pose.qWorldFromDriverRotation, tmpConj, pose.vecPosition, true) - pose.vecWorldFromDriverTranslation;
	auto poseWorldRot = tmpConj * pose.qRotation;

	// calculate orientation difference and its inverse
	_motionCompensationRotDiff = poseWorldRot * vrmath::quaternionConjugate(_motionCompensationZeroRot);
	_motionCompensationRotDiffInv = vrmath::quaternionConjugate(_motionCompensationRotDiff);

	// Convert velocity and acceleration values into app space and undo device rotation
	if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::SubstractMotionRef) {
		auto tmpRot = tmpConj * vrmath::quaternionConjugate(pose.qRotation);
		auto tmpRotInv = vrmath::quaternionConjugate(tmpRot);
		_motionCompensationRefPosVel = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, { pose.vecVelocity[0], pose.vecVelocity[1], pose.vecVelocity[2] });
		_motionCompensationRefPosAcc = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, { pose.vecAcceleration[0], pose.vecAcceleration[1], pose.vecAcceleration[2] });
		_motionCompensationRefRotVel = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, { pose.vecAngularVelocity[0], pose.vecAngularVelocity[1], pose.vecAngularVelocity[2] });
		_motionCompensationRefRotAcc = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, { pose.vecAngularAcceleration[0], pose.vecAngularAcceleration[1], pose.vecAngularAcceleration[2] });
		_motionCompensationRefVelAccValid = true;
	}

	_motionCompensationRefPoseValid = true;
}

bool ServerDriver::_applyMotionCompensation(vr::DriverPose_t& pose, DeviceManipulationHandle* deviceInfo) {
	if (_motionCompensationEnabled && _motionCompensationZeroPoseValid && _motionCompensationRefPoseValid) {
		// convert pose from driver space to app space
		vr::HmdQuaternion_t tmpConj = vrmath::quaternionConjugate(pose.qWorldFromDriverRotation);
		auto poseWorldPos = vrmath::quaternionRotateVector(pose.qWorldFromDriverRotation, tmpConj, pose.vecPosition, true) - pose.vecWorldFromDriverTranslation;
		auto poseWorldRot = tmpConj * pose.qRotation;

		// do motion compensation
		auto compensatedPoseWorldPos = _motionCompensationZeroPos + vrmath::quaternionRotateVector(_motionCompensationRotDiff, _motionCompensationRotDiffInv, poseWorldPos - _motionCompensationRefPos, true);
		auto compensatedPoseWorldRot = _motionCompensationRotDiffInv * poseWorldRot;

		// Velocity / Acceleration Compensation
		vr::HmdVector3d_t compensatedPoseWorldVel;
		bool compensatedPoseWorldVelValid = false;
		bool setVelToZero = false;
		bool setAccToZero = false;
		bool setAngVelToZero = false;
		bool setAngAccToZero = false;

		auto now = std::chrono::duration_cast <std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::SetZero) {
			setVelToZero = true;
			setAccToZero = true;
			setAngVelToZero = true;
			setAngAccToZero = true;

		} else if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::SubstractMotionRef) {
			// We translate the motion ref vel/acc values into driver space and directly substract them
			if (_motionCompensationRefVelAccValid) {
				auto tmpRot = pose.qWorldFromDriverRotation * pose.qRotation;
				auto tmpRotInv = vrmath::quaternionConjugate(tmpRot);
				auto tmpPosVel = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, _motionCompensationRefPosVel);
				pose.vecVelocity[0] -= tmpPosVel.v[0];
				pose.vecVelocity[1] -= tmpPosVel.v[1];
				pose.vecVelocity[2] -= tmpPosVel.v[2];
				auto tmpPosAcc = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, _motionCompensationRefPosAcc);
				pose.vecAcceleration[0] -= tmpPosAcc.v[0];
				pose.vecAcceleration[1] -= tmpPosAcc.v[1];
				pose.vecAcceleration[2] -= tmpPosAcc.v[2];
				auto tmpRotVel = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, _motionCompensationRefRotVel);
				pose.vecAngularVelocity[0] -= tmpRotVel.v[0];
				pose.vecAngularVelocity[1] -= tmpRotVel.v[1];
				pose.vecAngularVelocity[2] -= tmpRotVel.v[2];
				auto tmpRotAcc = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, _motionCompensationRefRotAcc);
				pose.vecAngularAcceleration[0] -= tmpRotAcc.v[0];
				pose.vecAngularAcceleration[1] -= tmpRotAcc.v[1];
				pose.vecAngularAcceleration[2] -= tmpRotAcc.v[2];
			}

		} else if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::KalmanFilter) {
			// The Kalman filter uses app space coordinates
			auto lastTime = deviceInfo->getLastPoseTime();
			if (lastTime >= 0.0) {
				double tdiff = ((double)(now - lastTime) / 1.0E6) + (pose.poseTimeOffset - deviceInfo->getLastPoseTimeOffset());
				if (tdiff < 0.0001) { // Sometimes we get a very small or even negative time difference between current and last pose
									  // In this case we just take the velocities and accelerations from last time
					auto& lastPose = deviceInfo->lastDriverPose();
					pose.vecVelocity[0] = lastPose.vecVelocity[0];
					pose.vecVelocity[1] = lastPose.vecVelocity[1];
					pose.vecVelocity[2] = lastPose.vecVelocity[2];
					pose.vecAcceleration[0] = lastPose.vecAcceleration[0];
					pose.vecAcceleration[1] = lastPose.vecAcceleration[1];
					pose.vecAcceleration[2] = lastPose.vecAcceleration[2];
					pose.vecAngularVelocity[0] = lastPose.vecAngularVelocity[0];
					pose.vecAngularVelocity[1] = lastPose.vecAngularVelocity[1];
					pose.vecAngularVelocity[2] = lastPose.vecAngularVelocity[2];
					pose.vecAngularAcceleration[0] = lastPose.vecAngularAcceleration[0];
					pose.vecAngularAcceleration[1] = lastPose.vecAngularAcceleration[1];
					pose.vecAngularAcceleration[2] = lastPose.vecAngularAcceleration[2];
				} else {
					deviceInfo->kalmanFilter().update(compensatedPoseWorldPos, tdiff);
					//compensatedPoseWorldPos = deviceInfo->kalmanFilter().getUpdatedPositionEstimate(); // Better to use the original values
					compensatedPoseWorldVel = deviceInfo->kalmanFilter().getUpdatedVelocityEstimate();
					compensatedPoseWorldVelValid = true;
					// Kalman filter only gives us velocity, so set the rest to zero
					setAccToZero = true;
					setAngVelToZero = true;
					setAngAccToZero = true;
				}
			} else {
				deviceInfo->kalmanFilter().init(
					compensatedPoseWorldPos,
					{0.0, 0.0, 0.0},
					{ {0.0, 0.0}, {0.0, 0.0} }
				);
				deviceInfo->kalmanFilter().setProcessNoise(m_motionCompensationKalmanProcessVariance);
				deviceInfo->kalmanFilter().setObservationNoise(m_motionCompensationKalmanObservationVariance);
				// Kalman Filter is not ready yet, so set everything to zero
				setVelToZero = true;
				setAccToZero = true;
				setAngVelToZero = true;
				setAngAccToZero = true;
			}

		} else if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::LinearApproximation) {
			// Linear approximation uses driver space coordinates
			if (deviceInfo->lastDriverPoseValid()) {
				auto& lastPose = deviceInfo->lastDriverPose();
				double tdiff = ((double)(now - deviceInfo->getLastPoseTime()) / 1.0E6) + (pose.poseTimeOffset - lastPose.poseTimeOffset);
				if (tdiff < 0.0001) { // Sometimes we get a very small or even negative time difference between current and last pose
									  // In this case we just take the velocities and accelerations from last time
					pose.vecVelocity[0] = lastPose.vecVelocity[0];
					pose.vecVelocity[1] = lastPose.vecVelocity[1];
					pose.vecVelocity[2] = lastPose.vecVelocity[2];
					pose.vecAcceleration[0] = lastPose.vecAcceleration[0];
					pose.vecAcceleration[1] = lastPose.vecAcceleration[1];
					pose.vecAcceleration[2] = lastPose.vecAcceleration[2];
					pose.vecAngularVelocity[0] = lastPose.vecAngularVelocity[0];
					pose.vecAngularVelocity[1] = lastPose.vecAngularVelocity[1];
					pose.vecAngularVelocity[2] = lastPose.vecAngularVelocity[2];
					pose.vecAngularAcceleration[0] = lastPose.vecAngularAcceleration[0];
					pose.vecAngularAcceleration[1] = lastPose.vecAngularAcceleration[1];
					pose.vecAngularAcceleration[2] = lastPose.vecAngularAcceleration[2];
				} else {
					vr::HmdVector3d_t p;
					p.v[0] = (pose.vecPosition[0] - lastPose.vecPosition[0]) / tdiff;
					if (p.v[0] > -0.01 && p.v[0] < 0.01) { // Set very small values to zero to avoid jitter
						p.v[0] = 0.0;
					}
					p.v[1] = (pose.vecPosition[1] - lastPose.vecPosition[1]) / tdiff;
					if (p.v[1] > -0.01 && p.v[1] < 0.01) {
						p.v[1] = 0.0;
					}
					p.v[2] = (pose.vecPosition[2] - lastPose.vecPosition[2]) / tdiff;
					if (p.v[2] > -0.01 && p.v[2] < 0.01) {
						p.v[2] = 0.0;
					}
					deviceInfo->velMovingAverage().push(p);
					auto vel = deviceInfo->velMovingAverage().average();
					pose.vecVelocity[0] = vel.v[0];
					pose.vecVelocity[1] = vel.v[1];
					pose.vecVelocity[2] = vel.v[2];
					// Predicting acceleration values leads to a very jittery experience.
					// Also, the lighthouse driver does not send acceleration values any way, so why care?
					setAccToZero = true;
					setAngVelToZero = true;
					setAngAccToZero = true;
				}
			} else {
				// Linear approximation is not ready yet, so set everything to zero
				setVelToZero = true;
				setAccToZero = true;
				setAngVelToZero = true;
				setAngAccToZero = true;
			}
		}
		deviceInfo->setLastDriverPose(pose, now);

		// convert back to driver space
		pose.qRotation = pose.qWorldFromDriverRotation * compensatedPoseWorldRot;
		auto adjPoseDriverPos = vrmath::quaternionRotateVector(pose.qWorldFromDriverRotation, tmpConj, compensatedPoseWorldPos + pose.vecWorldFromDriverTranslation);
		pose.vecPosition[0] = adjPoseDriverPos.v[0];
		pose.vecPosition[1] = adjPoseDriverPos.v[1];
		pose.vecPosition[2] = adjPoseDriverPos.v[2];
		if (compensatedPoseWorldVelValid) {
			auto adjPoseDriverVel = vrmath::quaternionRotateVector(pose.qWorldFromDriverRotation, tmpConj, compensatedPoseWorldVel);
			pose.vecVelocity[0] = adjPoseDriverVel.v[0];
			pose.vecVelocity[1] = adjPoseDriverVel.v[1];
			pose.vecVelocity[2] = adjPoseDriverVel.v[2];
		} else if (setVelToZero) {
			pose.vecVelocity[0] = 0.0;
			pose.vecVelocity[1] = 0.0;
			pose.vecVelocity[2] = 0.0;
		}
		if (setAccToZero) {
			pose.vecAcceleration[0] = 0.0;
			pose.vecAcceleration[1] = 0.0;
			pose.vecAcceleration[2] = 0.0;
		}
		if (setAngVelToZero) {
			pose.vecAngularVelocity[0] = 0.0;
			pose.vecAngularVelocity[1] = 0.0;
			pose.vecAngularVelocity[2] = 0.0;
		}
		if (setAngAccToZero) {
			pose.vecAngularAcceleration[0] = 0.0;
			pose.vecAngularAcceleration[1] = 0.0;
			pose.vecAngularAcceleration[2] = 0.0;
		}

		return true;
	} else {
		return true;
	}
}*/


void ServerDriver::sendReplySetMotionCompensationMode(bool success) {
	shmCommunicator.sendReplySetMotionCompensationMode(success);
}

} // end namespace driver
} // end namespace vrinputemulator
