#pragma once

#include <Leap.h>
#include <vrinputemulator.h>
#include <openvr_math.h>


class LeapMotionClient : public Leap::Listener {
private:
	Leap::Controller* leapController;
	vrinputemulator::VRInputEmulator* inputEmulator;


	uint32_t virtualIdLeft;
	bool leftReadyFlag = false;
	vr::DriverPose_t leftPose;
	vr::VRControllerState_t leftState;
	double leftTranslationOffset[3] = {0.0, 0.0, -0.15};
	vr::HmdQuaternion_t leftRotationOffset = vrmath::quaternionFromYawPitchRoll(0.0f, -0.1f, 0.0f);

	uint32_t virtualIdRight;
	bool rightReadyFlag = false;
	vr::DriverPose_t rightPose;
	vr::VRControllerState_t rightState;
	double rightTranslationOffset[3] = { 0.0, 0.0, -0.15 };
	vr::HmdQuaternion_t rightRotationOffset = vrmath::quaternionFromYawPitchRoll(0.0f, -0.1f, 0.0f);

public:
	LeapMotionClient(Leap::Controller* leapController, vrinputemulator::VRInputEmulator* inputEmulator);
	~LeapMotionClient();

	virtual void onFrame(const Leap::Controller &) override;
};

