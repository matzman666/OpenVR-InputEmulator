#pragma once
#pragma once

#include <openvr_driver.h>
#include <vrinputemulator_types.h>
#include <openvr_math.h>
#include "utils/KalmanFilter.h"
#include "utils/MovingAverageRingBuffer.h"
#include "../logging.h"
#include "../hooks/common.h"



// driver namespace
namespace vrinputemulator {
namespace driver {


// forward declarations
class ServerDriver;
class InterfaceHooks;
class MotionCompensationManager;


// Stores manipulation information about an openvr device
class DeviceManipulationHandle {
private:
	bool m_isValid = false;
	ServerDriver* m_parent;
	MotionCompensationManager& m_motionCompensationManager;
	std::recursive_mutex _mutex;
	vr::ETrackedDeviceClass m_eDeviceClass = vr::TrackedDeviceClass_Invalid;
	uint32_t m_openvrId = vr::k_unTrackedDeviceIndexInvalid;
	std::string m_serialNumber;

	int m_deviceDriverInterfaceVersion = 0;
	void* m_deviceDriverPtr;
	void* m_deviceDriverHostPtr;
	void* m_driverInputPtr = nullptr;
	std::shared_ptr<InterfaceHooks> m_serverDriverHooks;
	std::shared_ptr<InterfaceHooks> m_controllerComponentHooks;

	int m_deviceMode = 0; // 0 .. default, 1 .. disabled, 2 .. redirect source, 3 .. redirect target, 4 .. swap mode, 5 .. motion compensation
	bool _disconnectedMsgSend = false;

	bool m_offsetsEnabled = false;
	vr::HmdQuaternion_t m_worldFromDriverRotationOffset = { 1.0, 0.0, 0.0, 0.0 };
	vr::HmdVector3d_t m_worldFromDriverTranslationOffset = { 0.0, 0.0, 0.0 };
	vr::HmdQuaternion_t m_driverFromHeadRotationOffset = { 1.0, 0.0, 0.0, 0.0 };
	vr::HmdVector3d_t m_driverFromHeadTranslationOffset = { 0.0, 0.0, 0.0 };
	vr::HmdQuaternion_t m_deviceRotationOffset = { 1.0, 0.0, 0.0, 0.0 };
	vr::HmdVector3d_t m_deviceTranslationOffset = { 0.0, 0.0, 0.0 };

	struct DigitalInputRemappingInfo {
		int state = 0;
		std::chrono::system_clock::time_point timeout;
		struct BindingInfo {
			int state = 0;
			std::chrono::system_clock::time_point timeout;
			bool pressedState = false;
			bool touchedState = false;
			bool touchedAutoset = false;
			bool autoTriggerEnabled = false;
			bool autoTriggerState = false;
			std::chrono::system_clock::time_point autoTriggerTimeout;
			std::chrono::system_clock::time_point autoTriggerUnpressTimeout;
			uint32_t autoTriggerTimeoutTime;
		} bindings[3]; // 0 .. normal, 1 .. long press, 2 .. double press
		DigitalInputRemapping remapping;
	};
	std::map<uint32_t, DigitalInputRemappingInfo> m_digitalInputRemapping;

	struct AnalogInputRemappingInfo {
		struct BindingInfo {
			bool pressedState = false;
			bool touchedState = false;
			vr::VRControllerAxis_t lastSeenAxisState = { 0, 0 };
			vr::VRControllerAxis_t lastSendAxisState = { 0, 0 };
		} binding;
		AnalogInputRemapping remapping;
	};
	AnalogInputRemappingInfo m_analogInputRemapping[5];

	static bool touchpadEmulationEnabledFlag;

	bool m_redirectSuspended = false;
	DeviceManipulationHandle* m_redirectRef = nullptr;

	long long m_lastPoseTime = -1;
	bool m_lastPoseValid = false;
	vr::DriverPose_t m_lastPose;
	MovingAverageRingBuffer m_velMovingAverageBuffer;
	double m_lastPoseTimeOffset = 0.0;
	PosKalmanFilter m_kalmanFilter;

	vr::PropertyContainerHandle_t m_propertyContainerHandle = vr::k_ulInvalidPropertyContainer;
	uint64_t m_inputHapticComponentHandle = 0; // Let's assume for now that there is only one haptic component
	std::map<uint64_t, std::pair<vr::EVRButtonId, int>> _componentHandleToButtonIdMap;
	std::map<vr::EVRButtonId, std::pair<uint64_t, uint64_t>> _ButtonIdToComponentHandleMap;
	std::map<uint64_t, std::pair<unsigned, unsigned>> _componentHandleToAxisIdMap;
	std::pair<uint64_t, uint64_t> _AxisIdToComponentHandleMap[5];

	HANDLE _vibrationCueTheadHandle = NULL;

	void sendDigitalBinding(vrinputemulator::DigitalBinding& binding, uint32_t unWhichDevice, ButtonEventType eventType, vr::EVRButtonId eButtonId, double eventTimeOffset, DigitalInputRemappingInfo::BindingInfo* bindingInfo = nullptr);
	void sendAnalogBinding(vrinputemulator::AnalogBinding& binding, uint32_t unWhichDevice, uint32_t axisId, const vr::VRControllerAxis_t& axisState, AnalogInputRemappingInfo::BindingInfo* bindingInfo = nullptr);
	void sendAnalogBinding(vrinputemulator::AnalogBinding& binding, uint32_t unWhichDevice, uint32_t unWhichAxis, uint32_t unAxisDim, vr::VRInputComponentHandle_t ulComponent, float fNewValue, double fTimeOffset);

	void _buttonPressDeadzoneFix(vr::EVRButtonId eButtonId);
	void _vibrationCue();
	void _audioCue();

	int _disableOldMode(int newMode);

public:
	DeviceManipulationHandle(const char* serial, vr::ETrackedDeviceClass eDeviceClass, void* driverPtr, void* driverHostPtr, int driverInterfaceVersion);

	bool isValid() const { return m_isValid; }
	vr::ETrackedDeviceClass deviceClass() const { return m_eDeviceClass; }
	uint32_t openvrId() const { return m_openvrId; }
	void setOpenvrId(uint32_t id) { m_openvrId = id; }
	const std::string& serialNumber() { return m_serialNumber; }

	void* driverPtr() const { return m_deviceDriverPtr; }
	void* driverHostPtr() const { return m_deviceDriverHostPtr; }
	void* driverInputPtr() const { return m_driverInputPtr; }
	void setDriverInputPtr(void* ptr) { m_driverInputPtr = ptr; }
	void setServerDriverHooks(std::shared_ptr<InterfaceHooks> hooks) { m_serverDriverHooks = hooks; }
	void setControllerComponentHooks(std::shared_ptr<InterfaceHooks> hooks) { m_controllerComponentHooks = hooks; }

	int deviceMode() const { return m_deviceMode; }
	int setDefaultMode();
	int setRedirectMode(bool target, DeviceManipulationHandle* ref);
	int setSwapMode(DeviceManipulationHandle* ref);
	int setMotionCompensationMode();
	int setFakeDisconnectedMode();

	bool areOffsetsEnabled() const { return m_offsetsEnabled; }
	void enableOffsets(bool enable) { m_offsetsEnabled = enable; }
	const vr::HmdQuaternion_t& worldFromDriverRotationOffset() const { return m_worldFromDriverRotationOffset; }
	vr::HmdQuaternion_t& worldFromDriverRotationOffset() { return m_worldFromDriverRotationOffset; }
	const vr::HmdVector3d_t& worldFromDriverTranslationOffset() const { return m_worldFromDriverTranslationOffset; }
	vr::HmdVector3d_t& worldFromDriverTranslationOffset() { return m_worldFromDriverTranslationOffset; }
	const vr::HmdQuaternion_t& driverFromHeadRotationOffset() const { return m_driverFromHeadRotationOffset; }
	vr::HmdQuaternion_t& driverFromHeadRotationOffset() { return m_driverFromHeadRotationOffset; }
	const vr::HmdVector3d_t& driverFromHeadTranslationOffset() const { return m_driverFromHeadTranslationOffset; }
	vr::HmdVector3d_t& driverFromHeadTranslationOffset() { return m_driverFromHeadTranslationOffset; }
	const vr::HmdQuaternion_t& deviceRotationOffset() const { return m_deviceRotationOffset; }
	vr::HmdQuaternion_t& deviceRotationOffset() { return m_deviceRotationOffset; }
	const vr::HmdVector3d_t& deviceTranslationOffset() const { return m_deviceTranslationOffset; }
	vr::HmdVector3d_t& deviceTranslationOffset() { return m_deviceTranslationOffset; }

	void setDigitalInputRemapping(uint32_t buttonId, const DigitalInputRemapping& remapping);
	DigitalInputRemapping getDigitalInputRemapping(uint32_t buttonId);

	void setAnalogInputRemapping(uint32_t axisId, const AnalogInputRemapping& remapping);
	AnalogInputRemapping getAnalogInputRemapping(uint32_t axisId);

	bool redirectSuspended() const { return m_redirectSuspended; }
	DeviceManipulationHandle* redirectRef() const { return m_redirectRef; }

	void ll_sendPoseUpdate(const vr::DriverPose_t& newPose);
	void ll_sendButtonEvent(ButtonEventType eventType, vr::EVRButtonId eButtonId, double eventTimeOffset);
	void ll_sendAxisEvent(uint32_t unWhichAxis, const vr::VRControllerAxis_t& axisState);
	void ll_sendScalarComponentUpdate(vr::VRInputComponentHandle_t ulComponent, float fNewValue, double fTimeOffset);
	bool ll_triggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds);
	bool ll_sendHapticPulseEvent(float fDurationSeconds, float fFrequency, float fAmplitude);

	bool handlePoseUpdate(uint32_t& unWhichDevice, vr::DriverPose_t& newPose, uint32_t unPoseStructSize);
	bool handleButtonEvent(uint32_t& unWhichDevice, ButtonEventType eventType, vr::EVRButtonId& eButtonId, double& eventTimeOffset);
	bool handleAxisUpdate(uint32_t& unWhichDevice, uint32_t& unWhichAxis, vr::VRControllerAxis_t& axisState);
	bool handleBooleanComponentUpdate(vr::VRInputComponentHandle_t& ulComponent, bool& bNewValue, double& fTimeOffset);
	bool handleScalarComponentUpdate(vr::VRInputComponentHandle_t& ulComponent, float& fNewValue, double& fTimeOffset);
	bool handleHapticPulseEvent(float& fDurationSeconds, float& fFrequency, float& fAmplitude);

	void sendButtonEvent(uint32_t unWhichDevice, ButtonEventType eventType, vr::EVRButtonId eButtonId, double eventTimeOffset, bool directMode = false, DigitalInputRemappingInfo::BindingInfo* binding = nullptr);
	void sendKeyboardEvent(ButtonEventType eventType, bool shiftPressed, bool ctrlPressed, bool altPressed, WORD keyCode, bool sendScanCode, DigitalInputRemappingInfo::BindingInfo* binding = nullptr);
	void sendAxisEvent(uint32_t unWhichDevice, uint32_t unWhichAxis, const vr::VRControllerAxis_t& axisState, bool directMode = false, AnalogInputRemappingInfo::BindingInfo* binding = nullptr);
	void sendScalarComponentUpdate(uint32_t unWhichDevice, uint32_t unWhichAxis, uint32_t unAxisDim, vr::VRInputComponentHandle_t ulComponent, float fNewValue, double fTimeOffset, bool directMode = false);
	void sendScalarComponentUpdate(uint32_t unWhichDevice, uint32_t unWhichAxis, uint32_t unAxisDim, float, double fTimeOffset, bool directMode = false);
	

	bool triggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds, bool directMode = false);

	void inputAddBooleanComponent(const char *pchName, uint64_t pHandle);
	void inputAddScalarComponent(const char *pchName, uint64_t pHandle, vr::EVRScalarType eType, vr::EVRScalarUnits eUnits);
	void inputAddHapticComponent(const char * pchName, uint64_t pHandle);

	PosKalmanFilter& kalmanFilter() { return m_kalmanFilter; }
	MovingAverageRingBuffer& velMovingAverage() { return m_velMovingAverageBuffer; }
	long long getLastPoseTime() { return m_lastPoseTime; }
	void setLastPoseTime(long long time) { m_lastPoseTime = time; }
	double getLastPoseTimeOffset() { return m_lastPoseTimeOffset; }
	void setLastPoseTimeOffset(double offset) { m_lastPoseTimeOffset = offset; }
	bool lastDriverPoseValid() { return m_lastPoseValid; }
	vr::DriverPose_t& lastDriverPose() { return m_lastPose; }
	void setLastDriverPoseValid(bool valid) { m_lastPoseValid = valid; }
	void setLastDriverPose(const vr::DriverPose_t& pose, long long time) {
		m_lastPose = pose;
		m_lastPoseTime = time;
		m_lastPoseValid = true;
	}

	void setPropertyContainer(vr::PropertyContainerHandle_t container) { m_propertyContainerHandle = container; }
	vr::PropertyContainerHandle_t propertyContainer() { return m_propertyContainerHandle; }

	void RunFrame();
	void RunFrameDigitalBinding(vrinputemulator::DigitalBinding& binding, vr::EVRButtonId eButtonId, DigitalInputRemappingInfo::BindingInfo& bindingInfo);

	void suspendRedirectMode();

	static bool getTouchpadEmulationFixFlag() {
		return touchpadEmulationEnabledFlag;
	}
	static void setTouchpadEmulationFixFlag(bool flag) {
		touchpadEmulationEnabledFlag = flag;
	}
};


} // end namespace driver
} // end namespace vrinputemulator
