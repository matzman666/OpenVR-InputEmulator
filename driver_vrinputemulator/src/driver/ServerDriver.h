#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <openvr_driver.h>
#include <vrinputemulator_types.h>
#include <openvr_math.h>
#include "../hooks/common.h"
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

	//// function hooks related ////
	void hooksTrackedDeviceAdded(void* serverDriverHost, int version, const char *pchDeviceSerialNumber, vr::ETrackedDeviceClass& eDeviceClass, void* pDriver);
	void hooksTrackedDeviceActivated(void* serverDriver, int version, uint32_t unObjectId);
	bool hooksTrackedDevicePoseUpdated(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::DriverPose_t& newPose, uint32_t& unPoseStructSize);
	bool hooksTrackedDeviceButtonPressed(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::EVRButtonId& eButtonId, double& eventTimeOffset);
	bool hooksTrackedDeviceButtonUnpressed(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::EVRButtonId& eButtonId, double& eventTimeOffset);
	bool hooksTrackedDeviceButtonTouched(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::EVRButtonId& eButtonId, double& eventTimeOffset);
	bool hooksTrackedDeviceButtonUntouched(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::EVRButtonId& eButtonId, double& eventTimeOffset);
	bool hooksTrackedDeviceAxisUpdated(void* serverDriverHost, int version, uint32_t& unWhichDevice, uint32_t& unWhichAxis, vr::VRControllerAxis_t& axisState);
	bool hooksPollNextEvent(void* serverDriverHost, int version, void* pEvent, uint32_t uncbVREvent);
	
	bool hooksControllerTriggerHapticPulse(void* controllerComponent, int version, uint32_t& unAxisId, uint16_t& usPulseDurationMicroseconds);
	
	void hooksPropertiesReadPropertyBatch(void* properties, int version, vr::PropertyContainerHandle_t ulContainer, void* pBatch, uint32_t unBatchEntryCount);
	void hooksPropertiesWritePropertyBatch(void* properties, int version, vr::PropertyContainerHandle_t ulContainer, void* pBatch, uint32_t unBatchEntryCount);
	
	void hooksCreateBooleanComponent(void* driverInput, int version, vr::PropertyContainerHandle_t ulContainer, const char *pchName, void* pHandle);
	void hooksCreateScalarComponent(void* driverInput, int version, vr::PropertyContainerHandle_t ulContainer, const char *pchName, void* pHandle, vr::EVRScalarType eType, vr::EVRScalarUnits eUnits);
	void hooksCreateHapticComponent(void* driverInput, int version, vr::PropertyContainerHandle_t ulContainer, const char *pchName, void* pHandle);
	bool hooksUpdateBooleanComponent(void* driverInput, int version, vr::VRInputComponentHandle_t& ulComponent, bool& bNewValue, double& fTimeOffset);
	bool hooksUpdateScalarComponent(void* driverInput, int version, vr::VRInputComponentHandle_t& ulComponent, float& fNewValue, double& fTimeOffset);

	// driver events injection
	void addDriverEventForInjection(void* serverDriverHost, std::shared_ptr<void> event, uint32_t size);
	std::pair<std::shared_ptr<void>, uint32_t> getDriverEventForInjection(void* serverDriverHost);


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
	std::map<uint64_t, DeviceManipulationHandle*> _inputComponentToDeviceManipulationHandleMap;

	//// motion compensation related ////
	MotionCompensationManager m_motionCompensation;

	//// function hooks related ////
	std::shared_ptr<InterfaceHooks> _driverContextHooks;

	// driver events injection
	std::mutex _driverEventInjectionMutex;
	std::map<void*, std::queue<std::pair<std::shared_ptr<void>, uint32_t>>> m_eventsToInjectQueues;

	// Device Property Overrides
	std::string _propertiesOverrideHmdManufacturer;
	std::string _propertiesOverrideHmdModel;
	std::string _propertiesOverrideHmdTrackingSystem;
	bool _propertiesOverrideGenericTrackerFakeController;
};


} // end namespace driver
} // end namespace vrinputemulator
