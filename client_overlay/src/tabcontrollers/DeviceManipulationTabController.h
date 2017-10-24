
#pragma once

#include <QObject>
#include <memory>
#include <openvr.h>
#include <vrinputemulator.h>

class QQuickWindow;
// application namespace
namespace inputemulator {

// forward declaration
class OverlayController;


struct DigitalInputRemappingProfile {
	QString normalBindingControllerSerial;
	QString longBindingControllerSerial;
	QString doubleBindingControllerSerial;
	vrinputemulator::DigitalInputRemapping remapping;

	DigitalInputRemappingProfile() : remapping(true) {}
};

struct AnalogInputRemappingProfile {
	QString controllerSerial;
	vrinputemulator::AnalogInputRemapping remapping;

	AnalogInputRemappingProfile() : remapping(true) {}
};

struct DeviceManipulationProfile {
	std::string profileName;

	bool includesDeviceOffsets = false;
	bool deviceOffsetsEnabled = false;
	vr::HmdVector3d_t worldFromDriverTranslationOffset;
	vr::HmdVector3d_t worldFromDriverRotationOffset;
	vr::HmdVector3d_t driverFromHeadTranslationOffset;
	vr::HmdVector3d_t driverFromHeadRotationOffset;
	vr::HmdVector3d_t driverTranslationOffset;
	vr::HmdVector3d_t driverRotationOffset;
	bool includesInputRemapping = false;
	std::map<int, DigitalInputRemappingProfile> digitalRemappingProfiles;
	AnalogInputRemappingProfile analogRemappingProfiles[5];
};


struct DeviceInfo {
	std::string serial;
	vr::ETrackedDeviceClass deviceClass = vr::TrackedDeviceClass_Invalid;
	uint32_t openvrId = 0;
	int deviceStatus = 0; // 0 .. Normal, 1 .. Disconnected/Suspended
	int deviceMode = 0; // 0 .. Default, 1 .. Fake Disconnected, 2 .. Redirect Source, 3 .. Redirect Target, 4 .. Motion Compensation
	uint32_t refDeviceId = 0;
	bool deviceOffsetsEnabled;
	vr::HmdVector3d_t worldFromDriverRotationOffset;
	vr::HmdVector3d_t worldFromDriverTranslationOffset;
	vr::HmdVector3d_t driverFromHeadRotationOffset;
	vr::HmdVector3d_t driverFromHeadTranslationOffset;
	vr::HmdVector3d_t deviceRotationOffset;
	vr::HmdVector3d_t deviceTranslationOffset;
	uint32_t renderModelIndex = 0;
	vr::VROverlayHandle_t renderModelOverlay = vr::k_ulOverlayHandleInvalid;
	std::string renderModelOverlayName;
};


class DeviceManipulationTabController : public QObject {
	Q_OBJECT

private:
	OverlayController* parent;
	QQuickWindow* widget;

	std::vector<std::shared_ptr<DeviceInfo>> deviceInfos;
	uint32_t maxValidDeviceId = 0;

	std::vector<DeviceManipulationProfile> deviceManipulationProfiles;

	vrinputemulator::MotionCompensationVelAccMode motionCompensationVelAccMode = vrinputemulator::MotionCompensationVelAccMode::Disabled;
	double motionCompensationKalmanProcessNoise = 0.1;
	double motionCompensationKalmanObservationNoise = 0.1;
	unsigned motionCompensationMovingAverageWindow = 3;

	QString m_deviceModeErrorString;

	unsigned settingsUpdateCounter = 0;

	std::thread identifyThread;

public:
	~DeviceManipulationTabController();
	void initStage1();
	void initStage2(OverlayController* parent, QQuickWindow* widget);

	void eventLoopTick(vr::TrackedDevicePose_t* devicePoses);
	void handleEvent(const vr::VREvent_t& vrEvent);

	Q_INVOKABLE unsigned getDeviceCount();
	Q_INVOKABLE QString getDeviceSerial(unsigned index);
	Q_INVOKABLE unsigned getDeviceId(unsigned index);
	Q_INVOKABLE int getDeviceClass(unsigned index);
	Q_INVOKABLE int getDeviceState(unsigned index);
	Q_INVOKABLE int getDeviceMode(unsigned index);
	Q_INVOKABLE int getDeviceModeRefDeviceIndex(unsigned index);
	Q_INVOKABLE bool deviceOffsetsEnabled(unsigned index);
	Q_INVOKABLE double getWorldFromDriverRotationOffset(unsigned index, unsigned axis);
	Q_INVOKABLE double getWorldFromDriverTranslationOffset(unsigned index, unsigned axis);
	Q_INVOKABLE double getDriverFromHeadRotationOffset(unsigned index, unsigned axis);
	Q_INVOKABLE double getDriverFromHeadTranslationOffset(unsigned index, unsigned axis);
	Q_INVOKABLE double getDriverRotationOffset(unsigned index, unsigned axis);
	Q_INVOKABLE double getDriverTranslationOffset(unsigned index, unsigned axis);
	Q_INVOKABLE unsigned getMotionCompensationVelAccMode();
	Q_INVOKABLE double getMotionCompensationKalmanProcessNoise();
	Q_INVOKABLE double getMotionCompensationKalmanObservationNoise();
	Q_INVOKABLE unsigned getMotionCompensationMovingAverageWindow();

	void reloadDeviceManipulationSettings();
	void reloadDeviceManipulationProfiles();
	void saveDeviceManipulationSettings();
	void saveDeviceManipulationProfiles();

	Q_INVOKABLE unsigned getDeviceManipulationProfileCount();
	Q_INVOKABLE QString getDeviceManipulationProfileName(unsigned index);

	Q_INVOKABLE unsigned getRenderModelCount();
	Q_INVOKABLE QString getRenderModelName(unsigned index);
	Q_INVOKABLE bool updateDeviceInfo(unsigned index);

	Q_INVOKABLE unsigned getDigitalButtonCount(unsigned deviceIndex);
	Q_INVOKABLE int getDigitalButtonId(unsigned deviceIndex, unsigned buttonIndex);
	Q_INVOKABLE QString getDigitalButtonName(unsigned deviceIndex, unsigned buttonId);
	Q_INVOKABLE QString getDigitalButtonStatus(unsigned deviceIndex, unsigned buttonId);

	Q_INVOKABLE unsigned getAnalogAxisCount(unsigned deviceIndex);
	Q_INVOKABLE int getAnalogAxisId(unsigned deviceIndex, unsigned axisIndex);
	Q_INVOKABLE QString getAnalogAxisName(unsigned deviceIndex, unsigned axisId);
	Q_INVOKABLE QString getAnalogAxisStatus(unsigned deviceIndex, unsigned axisId);

	Q_INVOKABLE void startConfigureDigitalInputRemapping(unsigned deviceIndex, unsigned buttonId);
	Q_INVOKABLE void finishConfigureDigitalInputRemapping(unsigned deviceIndex, unsigned buttonId, bool touchAsClick, 
		bool longPress, int longPressThreshold, bool longPressImmediateRelease, bool doublePress, int doublePressThreshold, bool doublePressImmediateRelease);

	Q_INVOKABLE void startConfigureAnalogInputRemapping(unsigned deviceIndex, unsigned axisId);
	Q_INVOKABLE void finishConfigureAnalogInputRemapping(unsigned deviceIndex, unsigned axisId);

	Q_INVOKABLE bool setDeviceMode(unsigned index, unsigned mode, unsigned targedIndex, bool notify = true);
	Q_INVOKABLE QString getDeviceModeErrorString();


public slots:
	void enableDeviceOffsets(unsigned index, bool enable, bool notify = true);
	void setWorldFromDriverRotationOffset(unsigned index, double x, double y, double z, bool notify = true);
	void setWorldFromDriverTranslationOffset(unsigned index, double yaw, double pitch, double roll, bool notify = true);
	void setDriverFromHeadRotationOffset(unsigned index, double x, double y, double z, bool notify = true);
	void setDriverFromHeadTranslationOffset(unsigned index, double yaw, double pitch, double roll, bool notify = true);
	void setDriverRotationOffset(unsigned index, double x, double y, double z, bool notify = true);
	void setDriverTranslationOffset(unsigned index, double yaw, double pitch, double roll, bool notify = true);
	void triggerHapticPulse(unsigned index);
	void setDeviceRenderModel(unsigned deviceIndex, unsigned renderModelIndex);

	void addDeviceManipulationProfile(QString name, unsigned deviceIndex, bool includesDeviceOffsets, bool includesInputRemapping);
	void applyDeviceManipulationProfile(unsigned index, unsigned deviceIndex);
	void deleteDeviceManipulationProfile(unsigned index);

	void setMotionCompensationVelAccMode(unsigned mode, bool notify = true);
	void setMotionCompensationKalmanProcessNoise(double variance, bool notify = true);
	void setMotionCompensationKalmanObservationNoise(double variance, bool notify = true);
	void setMotionCompensationMovingAverageWindow(unsigned window, bool notify = true);

signals:
	void deviceCountChanged(unsigned deviceCount);
	void deviceInfoChanged(unsigned index);
	void motionCompensationSettingsChanged();
	void deviceManipulationProfilesChanged();
	void motionCompensationVelAccModeChanged(unsigned mode);
	void motionCompensationKalmanProcessNoiseChanged(double variance);
	void motionCompensationKalmanObservationNoiseChanged(double variance);
	void motionCompensationMovingAverageWindowChanged(unsigned window);

	void configureDigitalInputRemappingFinished();
	void configureAnalogInputRemappingFinished();
};

} // namespace inputemulator
