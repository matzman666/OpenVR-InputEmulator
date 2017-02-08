
#include <iostream>
#include <Leap.h>
#include <openvr.h>
#include <thread>
#include <vrinputemulator.h>
#include "LeapMotionClient.h"


int main() {

	Leap::Controller leapController;
	leapController.setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);
	leapController.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
	if (leapController.isServiceConnected()) {
		std::cout << "Leap Motion Service not running." << std::endl;
		exit(-1);
	}

	vr::EVRInitError peError;
	vr::VR_Init(&peError, vr::VRApplication_Overlay);
	if (peError != vr::VRInitError_None) {
		std::cout << "OpenVR Error: " << vr::VR_GetVRInitErrorAsEnglishDescription(peError) << std::endl;
		exit(-2);
	}

	vrinputemulator::VRInputEmulator inputEmulator;
	try {
		inputEmulator.connect();
	} catch (std::exception& e) {
		std::cout << "Caught exception: " << e.what() << std::endl;
		exit(-3);
	}

	LeapMotionClient leapClient(&leapController, &inputEmulator);
	leapController.addListener(leapClient);

	vr::VREvent_t vrevent;
	bool stopLoop = false;
	while(!stopLoop) {
		if (vr::VRSystem()->PollNextEvent(&vrevent, sizeof(vr::VREvent_t))) {
			if (vrevent.eventType == vr::VREvent_Quit) {
				stopLoop = true;
			}
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	leapController.removeListener(leapClient);
	vr::VR_Shutdown();

	return 0;
}

