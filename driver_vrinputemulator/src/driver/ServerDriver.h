#pragma once

#include <memory>
#include <mutex>
#include <openvr_driver.h>
#include <vrinputemulator_types.h>
#include <openvr_math.h>
#include "../logging.h"
#include "../com/shm/driver_ipc_shm.h"
#include "../devicemanipulation/MotionCompensationManager.h"



// driver namespace
namespace vrinputemulator {
namespace driver {


// forward declarations
class ServerDriver;
class InterfaceHooks;
class VirtualDeviceDriver;
class DeviceManipulationHandle;


/**
* Implements the IServerTrackedDeviceProvider interface.
*
* Its the main entry point of the driver. It's a singleton which manages all devices owned by this driver, 
* and also handles the whole "hacking into OpenVR" stuff.
*/
class ServerDriver : public vr::IServerTrackedDeviceProvider {
public:
	ServerDriver();
	virtual ~ServerDriver();

	//// from IServerTrackedDeviceProvider ////

	/** initializes the driver. This will be called before any other methods are called. */
	virtual vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext) override;

	/** cleans up the driver right before it is unloaded */
	virtual void Cleanup() override;

	/** Returns the version of the ITrackedDeviceServerDriver interface used by this driver */
	virtual const char * const *GetInterfaceVersions() { return vr::k_InterfaceVersions; }

	/** Allows the driver do to some work in the main loop of the server. Call frequency seems to be around 90Hz. */
	virtual void RunFrame() override;

	/** Returns true if the driver wants to block Standby mode. */
	virtual bool ShouldBlockStandbyMode() override { return false; }

	/** Called when the system is entering Standby mode */
	virtual void EnterStandby() override {}

	/** Called when the system is leaving Standby mode */
	virtual void LeaveStandby() override {}


	//// self ////

	static ServerDriver* getInstance() { return singleton; }

	static std::string getInstallDirectory() { return installDir; }

	uint32_t virtualDevices_getDeviceCount();

	VirtualDeviceDriver* virtualDevices_getDevice(uint32_t unWhichDevice);

	VirtualDeviceDriver* virtualDevices_findDevice(const std::string& serial);

	int32_t virtualDevices_addDevice(VirtualDeviceType type, const std::string& serial);

	int32_t virtualDevices_publishDevice(uint32_t virtualDeviceId, bool notify = true);


	void openvr_buttonEvent(uint32_t unWhichDevice, ButtonEventType eventType, vr::EVRButtonId eButtonId, double eventTimeOffset);

	void openvr_axisEvent(uint32_t unWhichDevice, uint32_t unWhichAxis, const vr::VRControllerAxis_t& axisState);

	void openvr_poseUpdate(uint32_t unWhichDevice, vr::DriverPose_t& newPose, int64_t timestamp);

	void openvr_proximityEvent(uint32_t unWhichDevice, bool bProximitySensorTriggered);

	void openvr_vendorSpecificEvent(uint32_t unWhichDevice, vr::EVREventType eventType, vr::VREvent_Data_t & eventData, double eventTimeOffset);

	DeviceManipulationHandle* getDeviceManipulationHandleById(uint32_t unWhichDevice);
	DeviceManipulationHandle* getDeviceManipulationHandleByPropertyContainer(vr::PropertyContainerHandle_t container);


	// internal API

	void executeCodeForEachDeviceManipulationHandle(std::function<void(DeviceManipulationHandle*)> code) {
		for (auto d : _deviceManipulationHandles) {
			code(d.second.get());
		}
	}

	/** Called by virtual devices when they are activated */
	void _trackedDeviceActivated(uint32_t deviceId, VirtualDeviceDriver* device);

	/** Called by virtual devices when they are deactivated */
	void _trackedDeviceDeactivated(uint32_t deviceId);

	/* Motion Compensation related */
	MotionCompensationManager& motionCompensation() { return m_motionCompensation; }
	void sendReplySetMotionCompensationMode(bool success);

	/*void enableMotionCompensation(bool enable);
	MotionCompensationStatus motionCompensationStatus() { return _motionCompensationStatus; }
	void _setMotionCompensationStatus(MotionCompensationStatus status) { _motionCompensationStatus = status;  }
	void setMotionCompensationRefDevice(DeviceManipulationHandle* device);
	DeviceManipulationHandle* getMotionCompensationRefDevice();
	void setMotionCompensationVelAccMode(MotionCompensationVelAccMode velAccMode);
	double motionCompensationKalmanProcessVariance() { return m_motionCompensationKalmanProcessVariance; }
	void setMotionCompensationKalmanProcessVariance(double variance);
	double motionCompensationKalmanObservationVariance() { return m_motionCompensationKalmanObservationVariance; }
	void setMotionCompensationKalmanObservationVariance(double variance);
	double motionCompensationMovingAverageWindow() { return m_motionCompensationMovingAverageWindow; }
	void setMotionCompensationMovingAverageWindow(unsigned window);
	void _disableMotionCompensationOnAllDevices();
	bool _isMotionCompensationZeroPoseValid();
	void _setMotionCompensationZeroPose(const vr::DriverPose_t& pose);
	void _updateMotionCompensationRefPose(const vr::DriverPose_t& pose);
	bool _applyMotionCompensation(vr::DriverPose_t& pose, DeviceManipulationHandle* deviceInfo);
	void sendReplySetMotionCompensationMode(bool success);*/

	//// function hooks related ////

	void hooksTrackedDeviceAdded(void* serverDriverHost, int version, const char *pchDeviceSerialNumber, vr::ETrackedDeviceClass& eDeviceClass, void* pDriver);
	void hooksTrackedDeviceActivated(void* serverDriver, int version, uint32_t unObjectId);
	bool hooksTrackedDevicePoseUpdated(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::DriverPose_t& newPose, uint32_t& unPoseStructSize);
	bool hooksTrackedDeviceButtonPressed(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::EVRButtonId& eButtonId, double& eventTimeOffset);
	bool hooksTrackedDeviceButtonUnpressed(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::EVRButtonId& eButtonId, double& eventTimeOffset);
	bool hooksTrackedDeviceButtonTouched(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::EVRButtonId& eButtonId, double& eventTimeOffset);
	bool hooksTrackedDeviceButtonUntouched(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::EVRButtonId& eButtonId, double& eventTimeOffset);
	bool hooksTrackedDeviceAxisUpdated(void* serverDriverHost, int version, uint32_t& unWhichDevice, uint32_t& unWhichAxis, vr::VRControllerAxis_t& axisState);
	bool hooksControllerTriggerHapticPulse(void* controllerComponent, int version, uint32_t& unAxisId, uint16_t& usPulseDurationMicroseconds);
	void hooksPropertiesReadPropertyBatch(void* properties, vr::PropertyContainerHandle_t ulContainer, void* pBatch, uint32_t unBatchEntryCount);
	void hooksPropertiesWritePropertyBatch(void* properties, vr::PropertyContainerHandle_t ulContainer, void* pBatch, uint32_t unBatchEntryCount);


private:
	static ServerDriver* singleton;

	static std::string installDir;

	//// virtual devices related ////
	std::recursive_mutex _virtualDevicesMutex;
	uint32_t m_virtualDeviceCount = 0;
	std::shared_ptr<VirtualDeviceDriver> m_virtualDevices[vr::k_unMaxTrackedDeviceCount];
	VirtualDeviceDriver* m_openvrIdToVirtualDeviceMap[vr::k_unMaxTrackedDeviceCount];

	//// ipc shm related ////
	IpcShmCommunicator shmCommunicator;

	//// device manipulation related ////
	std::recursive_mutex _deviceManipulationHandlesMutex;
	std::map<void*, std::shared_ptr<DeviceManipulationHandle>> _deviceManipulationHandles;
	DeviceManipulationHandle* _openvrIdToDeviceManipulationHandleMap[vr::k_unMaxTrackedDeviceCount];
	std::map<vr::PropertyContainerHandle_t, DeviceManipulationHandle*> _propertyContainerToDeviceManipulationHandleMap;
	std::map<void*, DeviceManipulationHandle*> _ptrToDeviceManipulationHandleMap;

	//// motion compensation related ////
	MotionCompensationManager m_motionCompensation;
	/*bool _motionCompensationEnabled = false;
	DeviceManipulationHandle* _motionCompensationRefDevice = nullptr;
	MotionCompensationStatus _motionCompensationStatus = MotionCompensationStatus::WaitingForZeroRef;
	constexpr static uint32_t _motionCompensationZeroRefTimeoutMax = 20;
	uint32_t _motionCompensationZeroRefTimeout = 0;
	MotionCompensationVelAccMode _motionCompensationVelAccMode = MotionCompensationVelAccMode::Disabled;
	double m_motionCompensationKalmanProcessVariance = 0.1;
	double m_motionCompensationKalmanObservationVariance = 0.1;
	unsigned m_motionCompensationMovingAverageWindow = 3;

	bool _motionCompensationZeroPoseValid = false;
	vr::HmdVector3d_t _motionCompensationZeroPos;
	vr::HmdQuaternion_t _motionCompensationZeroRot;

	bool _motionCompensationRefPoseValid = false;
	vr::HmdVector3d_t _motionCompensationRefPos;
	vr::HmdQuaternion_t _motionCompensationRotDiff;
	vr::HmdQuaternion_t _motionCompensationRotDiffInv;

	bool _motionCompensationRefVelAccValid = false;
	vr::HmdVector3d_t _motionCompensationRefPosVel;
	vr::HmdVector3d_t _motionCompensationRefPosAcc;
	vr::HmdVector3d_t _motionCompensationRefRotVel;
	vr::HmdVector3d_t _motionCompensationRefRotAcc;*/

	//// function hooks related ////
	
	std::shared_ptr<InterfaceHooks> _driverContextHooks;


	// Device Property Overrides
	std::string _propertiesOverrideHmdManufacturer;
	std::string _propertiesOverrideHmdModel;
	std::string _propertiesOverrideHmdTrackingSystem;
	bool _propertiesOverrideGenericTrackerFakeController;
};


} // end namespace driver
} // end namespace vrinputemulator
