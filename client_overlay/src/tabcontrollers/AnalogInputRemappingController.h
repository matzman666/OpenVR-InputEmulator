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


class AnalogInputRemappingController : public QObject {
	Q_OBJECT

public:
	~AnalogInputRemappingController();
	void initStage1();
	void initStage2(OverlayController* parent, QQuickWindow* widget);

	void eventLoopTick(vr::TrackedDevicePose_t* devicePoses);
	void handleEvent(const vr::VREvent_t& vrEvent);

	void startConfigureRemapping(vrinputemulator::AnalogInputRemapping remapping, uint32_t deviceIndex, uint32_t deviceId, uint32_t axisId);

	Q_INVOKABLE int getAxisMaxCount();
	Q_INVOKABLE QString getAxisName(int id, bool withDefaults = true);

	Q_INVOKABLE int getBindingType();

	Q_INVOKABLE int getBindingOpenVRControllerId();
	Q_INVOKABLE int getBindingOpenVRAxisId();

	Q_INVOKABLE bool isBindingOpenVRXInverted();
	Q_INVOKABLE bool isBindingOpenVRYInverted();
	Q_INVOKABLE bool isBindingOpenVRAxesSwapped();

	Q_INVOKABLE float getBindingDeadzoneLower();
	Q_INVOKABLE float getBindingDeadzoneUpper();

	Q_INVOKABLE unsigned getBindingTouchpadEmulationMode();
	Q_INVOKABLE bool getBindingButtonPressDeadzoneFix();

	const vrinputemulator::AnalogInputRemapping& currentRemapping() { return m_currentRemapping; }

public slots:
	void finishConfigure_Original(unsigned touchpadEmulationMode, bool updateOnButtonEvent);
	void finishConfigure_Disabled();
	void finishConfigure_OpenVR(int controllerId, int axisId, bool invertXAxis, bool invertYAxis, bool swapAxes, float lowerDeadzone, float upperDeadzone, unsigned touchpadEmulationMode, bool updateOnButtonEvent);

private:
	OverlayController* parent;
	QQuickWindow* widget;

	unsigned settingsUpdateCounter = 0;

	vrinputemulator::AnalogInputRemapping m_currentRemapping;
	uint32_t m_currentDeviceIndex;
	uint32_t m_currentDeviceId;
	uint32_t m_currentAxisId;
};


} // namespace inputemulator
