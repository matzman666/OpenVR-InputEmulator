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

class DigitalInputRemappingController : public QObject {
	Q_OBJECT

public:
	~DigitalInputRemappingController();
	void initStage1();
	void initStage2(OverlayController* parent, QQuickWindow* widget);

	void eventLoopTick(vr::TrackedDevicePose_t* devicePoses);
	void handleEvent(const vr::VREvent_t& vrEvent);

	void startConfigureRemapping(vrinputemulator::DigitalInputRemapping remapping, uint32_t deviceIndex, uint32_t deviceId, uint32_t buttonId);
	Q_INVOKABLE void startConfigureNormalBinding();
	Q_INVOKABLE void startConfigureLongPressBinding();
	Q_INVOKABLE void startConfigureDoublePressBinding();

	Q_INVOKABLE QString getNormalBindingStatus();
	Q_INVOKABLE bool isLongPressEnabled();
	Q_INVOKABLE unsigned getLongPressThreshold();
	Q_INVOKABLE QString getLongPressBindingStatus();
	Q_INVOKABLE bool isLongPressImmediateRelease();

	Q_INVOKABLE bool isDoublePressEnabled();
	Q_INVOKABLE unsigned getDoublePressThreshold();
	Q_INVOKABLE QString getDoublePressBindingStatus();
	Q_INVOKABLE bool isDoublePressImmediateRelease();

	Q_INVOKABLE int getButtonMaxCount();
	Q_INVOKABLE QString getButtonName(int id, bool withDefaults = true);

	Q_INVOKABLE int getBindingType();

	Q_INVOKABLE int getBindingOpenVRControllerId();
	Q_INVOKABLE int getBindingOpenVRButtonId();

	Q_INVOKABLE bool touchAsClickEnabled();

	Q_INVOKABLE bool isToggleModeEnabled();
	Q_INVOKABLE int toggleModeThreshold();
	Q_INVOKABLE bool isAutoTriggerEnabled();
	Q_INVOKABLE int autoTriggerFrequency();

	Q_INVOKABLE bool keyboardShiftEnabled();
	Q_INVOKABLE bool keyboardCtrlEnabled();
	Q_INVOKABLE bool keyboardAltEnabled();
	Q_INVOKABLE unsigned keyboardKeyIndex();
	Q_INVOKABLE bool keyboardUseScanCode();

	const vrinputemulator::DigitalInputRemapping& currentRemapping() { return m_currentRemapping; }


public slots:
	/*void enableLongPress(bool enable, bool notify = true);
	void setLongPressThreshold(unsigned value, bool notify = true);
	void setLongPressImmediateRelease(unsigned value, bool notify = true);
	void enableDoublePress(bool enable, bool notify = true);
	void setDoublePressThreshold(unsigned value, bool notify = true);
	void setDoublePressImmediateRelease(unsigned value, bool notify = true);
	
	void enableTouchAsClick(bool enable, bool notify = true);*/

	void finishConfigureBinding_Original();
	void finishConfigureBinding_Disabled();
	void finishConfigureBinding_OpenVR(int controllerId, int ButtonId, bool toggleMode, int toggleThreshold, bool autoTrigger, int triggerFrequency);
	void finishConfigureBinding_keyboard(bool shiftPressed, bool ctrlPressed, bool altPressed, unsigned long keyIndex, bool useScanCode, bool toggleMode, int toggleThreshold, bool autoTrigger, int triggerFrequency);
	void finishConfigureBinding_suspendRedirectMode();
	void finishConfigureBinding_toggleTouchpadEmulationFix();

signals:
	/*void longPressEnabledChanged(bool enable);
	void longPressThresholdChanged(unsigned value);
	void longPressImmediateReleaseChanged(unsigned value);
	void doublePressEnabledChanged(bool enable);
	void doublePressThresholdChanged(unsigned value);
	void doublePressImmediateReleaseChanged(unsigned value);

	void touchAsClickChanged(bool value);*/

	void configureDigitalBindingFinished();

private:
	OverlayController* parent;
	QQuickWindow* widget;

	unsigned settingsUpdateCounter = 0;

	vrinputemulator::DigitalInputRemapping m_currentRemapping;
	uint32_t m_currentDeviceIndex;
	uint32_t m_currentDeviceId;
	uint32_t m_currentButtonId;
	vrinputemulator::DigitalBinding* m_currentBinding = nullptr;
};


} // namespace inputemulator
