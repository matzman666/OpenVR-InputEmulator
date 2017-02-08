#pragma once

#include "stdafx.h"
#include <openvr_driver.h>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <boost/variant.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <atomic>
#include "logging.h"
#include <vrinputemulator_types.h>


namespace vrinputemulator {
namespace driver {


// forward declarations
class CServerDriver;
class CClientDriver;
class CTrackedDeviceDriver;
class CTrackedControllerDriver;



class DevicePropertyValueVisitor : public boost::static_visitor<std::string> {
private:
	vr::PropertyContainerHandle_t propertyContainer;
	vr::ETrackedDeviceProperty deviceProperty;
public:
	DevicePropertyValueVisitor() = delete;
	DevicePropertyValueVisitor(vr::PropertyContainerHandle_t& propertyContainer, vr::ETrackedDeviceProperty deviceProperty)
		: propertyContainer(propertyContainer), deviceProperty(deviceProperty) {}
	template<class T>
	std::string operator()(T& i) const {
		return "Unknown value type";
	}
	template<>
	std::string operator()<int32_t>(int32_t& val) const {
		auto pError = vr::VRProperties()->SetInt32Property(propertyContainer, deviceProperty, val);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<uint64_t>(uint64_t& val) const {
		auto pError = vr::VRProperties()->SetUint64Property(propertyContainer, deviceProperty, val);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<float>(float& val) const {
		auto pError = vr::VRProperties()->SetFloatProperty(propertyContainer, deviceProperty, val);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<bool>(bool& val) const {
		auto pError = vr::VRProperties()->SetBoolProperty(propertyContainer, deviceProperty, val);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<std::string>(std::string& val) const {
		auto pError = vr::VRProperties()->SetStringProperty(propertyContainer, deviceProperty, val.c_str());
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<vr::HmdMatrix34_t>(vr::HmdMatrix34_t& val) const {
		auto pError = vr::VRProperties()->SetProperty(propertyContainer, deviceProperty, &val, sizeof(vr::HmdMatrix34_t), vr::k_unHmdMatrix34PropertyTag);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<vr::HmdMatrix44_t>(vr::HmdMatrix44_t& val) const {
		auto pError = vr::VRProperties()->SetProperty(propertyContainer, deviceProperty, &val, sizeof(vr::HmdMatrix44_t), vr::k_unHmdMatrix44PropertyTag);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<vr::HmdVector3_t>(vr::HmdVector3_t& val) const {
		auto pError = vr::VRProperties()->SetProperty(propertyContainer, deviceProperty, &val, sizeof(vr::HmdVector3_t), vr::k_unHmdVector3PropertyTag);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<vr::HmdVector4_t>(vr::HmdVector4_t& val) const {
		auto pError = vr::VRProperties()->SetProperty(propertyContainer, deviceProperty, &val, sizeof(vr::HmdVector4_t), vr::k_unHmdVector4PropertyTag);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
};



class CWatchdogProvider : public vr::IVRWatchdogProvider {
public:
	virtual vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext) override;
	virtual void Cleanup() override;
};


class CServerDriver : public vr::IServerTrackedDeviceProvider {
private:
	std::recursive_mutex _mutex;
	uint32_t m_emulatedDeviceCount = 0;
	uint32_t m_publishedDeviceCount = 0;
	std::shared_ptr<CTrackedDeviceDriver> m_emulatedDevices[vr::k_unMaxTrackedDeviceCount];
	CTrackedDeviceDriver* m_publishedDevices[vr::k_unMaxTrackedDeviceCount];
	CTrackedDeviceDriver* m_deviceIdMap[vr::k_unMaxTrackedDeviceCount];

	std::thread _ipcThread;
	volatile bool _ipcThreadRunning = false;
	volatile bool _ipcThreadStopFlag = false;
	std::string _ipcQueueName = "driver_vrinputemulator.server_queue";
	uint32_t _ipcClientIdNext = 1;
	std::map<uint32_t, std::shared_ptr<boost::interprocess::message_queue>> _ipcEndpoints;
	static void _ipcThreadFunc(CServerDriver* _this);

	template<class T>
	struct _DetourFuncInfo {
		bool enabled = false;
		void* targetFunc = nullptr;
		T origFunc = nullptr;
	};

	typedef bool(*_DetourTrackedDeviceAdded_t)(vr::IVRServerDriverHost*, const char*, vr::ETrackedDeviceClass, vr::ITrackedDeviceServerDriver*);
	typedef void(*_DetourTrackedDevicePoseUpdated_t)(vr::IVRServerDriverHost*, uint32_t, const vr::DriverPose_t&, uint32_t);
	typedef void(*_DetourTrackedDeviceButtonPressed_t)(vr::IVRServerDriverHost*, uint32_t, vr::EVRButtonId, double);
	typedef void(*_DetourTrackedDeviceButtonUnpressed_t)(vr::IVRServerDriverHost*, uint32_t, vr::EVRButtonId, double);
	typedef void(*_DetourTrackedDeviceButtonTouched_t)(vr::IVRServerDriverHost*, uint32_t, vr::EVRButtonId, double);
	typedef void(*_DetourTrackedDeviceButtonUntouched_t)(vr::IVRServerDriverHost*, uint32_t, vr::EVRButtonId, double);
	typedef void(*_DetourTrackedDeviceAxisUpdated_t)(vr::IVRServerDriverHost*, uint32_t, uint32_t, const vr::VRControllerAxis_t&);
	typedef vr::EVRInitError(*_DetourTrackedDeviceActivate_t)(vr::ITrackedDeviceServerDriver*, uint32_t);

	static CServerDriver* singleton;

	struct _TrackedDeviceInfo {
		_TrackedDeviceInfo() {}
		_TrackedDeviceInfo(vr::ITrackedDeviceServerDriver* driver, vr::ETrackedDeviceClass eDeviceClass, const std::string& serialNumber, uint32_t openvrId, vr::IVRServerDriverHost* driverHost)
				: isValid(true), driver(driver), eDeviceClass(eDeviceClass), serialNumber(serialNumber), openvrId(openvrId), driverHost(driverHost){}

		vr::IVRServerDriverHost* driverHost;

		bool isValid = false;
		vr::ITrackedDeviceServerDriver* driver = nullptr;
		vr::ETrackedDeviceClass eDeviceClass = vr::TrackedDeviceClass_Invalid;
		std::string serialNumber;
		uint32_t openvrId = vr::k_unTrackedDeviceIndexInvalid;

		bool enableButtonMapping = false;
		std::map<vr::EVRButtonId, vr::EVRButtonId> buttonMapping;

		bool enablePoseOffset = false;
		double poseOffset[3];

		bool enablePoseRotation = false;
		vr::HmdQuaternion_t poseRotation;

		uint32_t mirrorMode = 0; // 0 .. disabled, 1 .. mirror, 2 .. remap
		uint32_t mirrorTarget = vr::k_unTrackedDeviceIndexInvalid;

		bool getButtonMapping(vr::EVRButtonId button, vr::EVRButtonId& mappedButton) {
			if (enableButtonMapping) {
				auto i = buttonMapping.find(button);
				if (i != buttonMapping.end()) {
					mappedButton = i->second;
					return true;
				}
			}
			return false;
		}
	};

	static std::map<vr::ITrackedDeviceServerDriver*, _TrackedDeviceInfo> _trackedDeviceInfoMap;
	static _TrackedDeviceInfo* _trackedDeviceInfos[vr::k_unMaxTrackedDeviceCount]; // index == openvrId

	static _DetourFuncInfo<_DetourTrackedDeviceAdded_t> _deviceAddedDetour;
	static _DetourFuncInfo<_DetourTrackedDevicePoseUpdated_t> _poseUpatedDetour;
	static _DetourFuncInfo<_DetourTrackedDeviceButtonPressed_t> _buttonPressedDetour;
	static _DetourFuncInfo<_DetourTrackedDeviceButtonUnpressed_t> _buttonUnpressedDetour;
	static _DetourFuncInfo<_DetourTrackedDeviceButtonTouched_t> _buttonTouchedDetour;
	static _DetourFuncInfo<_DetourTrackedDeviceButtonUntouched_t> _buttonUntouchedDetour;
	static _DetourFuncInfo<_DetourTrackedDeviceAxisUpdated_t> _axisUpdatedDetour;
	static std::vector<_DetourFuncInfo<_DetourTrackedDeviceActivate_t>> _deviceActivateDetours;
	static std::map<vr::ITrackedDeviceServerDriver*, _DetourFuncInfo<_DetourTrackedDeviceActivate_t>*> _deviceActivateDetourMap; // _this => DetourInfo

	static void _poseUpatedDetourFunc(vr::IVRServerDriverHost* _this, uint32_t unWhichDevice, const vr::DriverPose_t& newPose, uint32_t unPoseStructSize);
	static void _buttonPressedDetourFunc(vr::IVRServerDriverHost* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);
	static void _buttonUnpressedDetourFunc(vr::IVRServerDriverHost* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);
	static void _buttonTouchedDetourFunc(vr::IVRServerDriverHost* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);
	static void _buttonUntouchedDetourFunc(vr::IVRServerDriverHost* _this, uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);
	static void _axisUpdatedDetourFunc(vr::IVRServerDriverHost* _this, uint32_t unWhichDevice, uint32_t unWhichAxis, const vr::VRControllerAxis_t & axisState);
	static vr::EVRInitError _deviceActivateDetourFunc(vr::ITrackedDeviceServerDriver* _this, uint32_t unObjectId);
	static bool _deviceAddedDetourFunc(vr::IVRServerDriverHost* _this, const char *pchDeviceSerialNumber, vr::ETrackedDeviceClass eDeviceClass, vr::ITrackedDeviceServerDriver *pDriver);

public:
	CServerDriver() {
		singleton = this;
		memset(m_deviceIdMap, 0, sizeof(CTrackedDeviceDriver*) * vr::k_unMaxTrackedDeviceCount);
		memset(_trackedDeviceInfos, 0, sizeof(_TrackedDeviceInfo*) * vr::k_unMaxTrackedDeviceCount);
	}
	virtual ~CServerDriver();

	// from IServerTrackedDeviceProvider

	virtual vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext) override;
	virtual void Cleanup() override;
	virtual const char * const *GetInterfaceVersions() { return vr::k_InterfaceVersions; }
	virtual void RunFrame() override;

	virtual bool ShouldBlockStandbyMode() override { return false; }
	virtual void EnterStandby() override {}
	virtual void LeaveStandby() override {}

	// from self


	int32_t addTrackedDevice(VirtualDeviceType type, const std::string& serial);
	int32_t publishTrackedDevice(uint32_t deviceId, bool notify = true);

	void trackedDeviceActivated(uint32_t deviceId, CTrackedDeviceDriver* device);
	void trackedDeviceDeactivated(uint32_t deviceId);
};



class CTrackedDeviceDriver : public vr::ITrackedDeviceServerDriver {
protected:
	std::recursive_mutex _mutex;

	CServerDriver* m_serverDriver;
	VirtualDeviceType m_deviceType;
	std::string m_serialNumber;
	uint32_t m_openvrDeviceId = vr::k_unTrackedDeviceIndexInvalid;
	vr::PropertyContainerHandle_t m_propertyContainer = vr::k_ulInvalidPropertyContainer;

	vr::DriverPose_t m_pose;
	typedef boost::variant<int32_t, uint64_t, float, bool, std::string, vr::HmdMatrix34_t, vr::HmdMatrix44_t, vr::HmdVector3_t, vr::HmdVector4_t> _devicePropertyType_t;
	std::map<int, _devicePropertyType_t> _deviceProperties;

public:
	CTrackedDeviceDriver(CServerDriver* parent, VirtualDeviceType type, const std::string& serial);

	// from ITrackedDeviceServerDriver

	virtual vr::EVRInitError Activate(uint32_t unObjectId) override;
	virtual void Deactivate() override;
	virtual void EnterStandby() override {}
	virtual void *GetComponent(const char *pchComponentNameAndVersion) override;
	virtual void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) override {}
	virtual vr::DriverPose_t GetPose() override;

	// from self

	const std::string& serialNumber() { return m_serialNumber; }
	VirtualDeviceType deviceType() { return m_deviceType; }

	CServerDriver* serverDriver() { return m_serverDriver; }
	uint32_t openvrDeviceId() { return m_openvrDeviceId; }

	vr::DriverPose_t& driverPose() { return m_pose; }

	void updatePose(const vr::DriverPose_t& newPose, double timeOffset, bool notify = true);

	template<class T>
	T getTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (pError) {
			*pError = vr::TrackedProp_Success;
		}
		auto p = _deviceProperties.find((int)prop);
		if (p != _deviceProperties.end()) {
			try {
				T retval = boost::get<T>(p->second);
				return retval;
			} catch (std::exception&) {
				if (pError) {
					*pError = vr::TrackedProp_WrongDataType;
				}
				return T();
			}
		} else {
			if (pError) {
				*pError = vr::TrackedProp_ValueNotProvidedByDevice;
			}
			return T();
		}
	}

	template<class T>
	void setTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, const T& value, bool notify = true) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		_deviceProperties[prop] = value;
		if (notify && m_openvrDeviceId != vr::k_unTrackedDeviceIndexInvalid) {
			auto errorMessage = boost::apply_visitor(DevicePropertyValueVisitor(m_propertyContainer, prop), _deviceProperties[prop]);
			if (!errorMessage.empty()) {
				LOG(ERROR) << "Could not set tracked device property: " << errorMessage;
			}
		}
	}

	void removeTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, bool notify = true) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		auto i = _deviceProperties.find(prop);
		if (i != _deviceProperties.end()) {
			_deviceProperties.erase(i);
			if (notify && m_openvrDeviceId != vr::k_unTrackedDeviceIndexInvalid) {
				vr::VRProperties()->EraseProperty(m_propertyContainer, prop);
			}
		}
	}
};



class CTrackedControllerDriver : public CTrackedDeviceDriver, public vr::IVRControllerComponent {
private:
	vr::VRControllerState_t m_ControllerState;

public:
	CTrackedControllerDriver(CServerDriver* parent, const std::string& serial);

	// from ITrackedDeviceServerDriver

	virtual void *GetComponent(const char *pchComponentNameAndVersion) override;

	// from IVRControllerComponent

	virtual vr::VRControllerState_t GetControllerState() override;
	virtual bool TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds) override;

	// from self

	vr::VRControllerState_t& controllerState() { return m_ControllerState; }

	void updateControllerState(const vr::VRControllerState_t& newState, double timeOffset, bool notify = true);
	void buttonEvent(ButtonEventType eventType, uint32_t buttonId, double timeOffset, bool notify = true);
	void axisEvent(uint32_t axisId, const vr::VRControllerAxis_t& axisState, bool notify = true);

};

} // end namespace driver
} // end namespace vrinputemulator
