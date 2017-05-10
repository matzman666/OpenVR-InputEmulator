#include "LeapMotionClient.h"
#include "utils/Matrix.h"
#include "openvr_math.h"



// convert a 3x3 rotation matrix into a rotation quaternion
static vr::HmdQuaternion_t CalculateRotation(float(*a)[3]) {

	vr::HmdQuaternion_t q;

	float trace = a[0][0] + a[1][1] + a[2][2];
	if (trace > 0) {
		float s = 0.5f / sqrtf(trace + 1.0f);
		q.w = 0.25f / s;
		q.x = (a[2][1] - a[1][2]) * s;
		q.y = (a[0][2] - a[2][0]) * s;
		q.z = (a[1][0] - a[0][1]) * s;
	} else {
		if (a[0][0] > a[1][1] && a[0][0] > a[2][2]) {
			float s = 2.0f * sqrtf(1.0f + a[0][0] - a[1][1] - a[2][2]);
			q.w = (a[2][1] - a[1][2]) / s;
			q.x = 0.25f * s;
			q.y = (a[0][1] + a[1][0]) / s;
			q.z = (a[0][2] + a[2][0]) / s;
		} else if (a[1][1] > a[2][2]) {
			float s = 2.0f * sqrtf(1.0f + a[1][1] - a[0][0] - a[2][2]);
			q.w = (a[0][2] - a[2][0]) / s;
			q.x = (a[0][1] + a[1][0]) / s;
			q.y = 0.25f * s;
			q.z = (a[1][2] + a[2][1]) / s;
		} else {
			float s = 2.0f * sqrtf(1.0f + a[2][2] - a[0][0] - a[1][1]);
			q.w = (a[1][0] - a[0][1]) / s;
			q.x = (a[0][2] + a[2][0]) / s;
			q.y = (a[1][2] + a[2][1]) / s;
			q.z = 0.25f * s;
		}
	}
	q.x = -q.x;
	q.y = -q.y;
	q.z = -q.z;
	return q;
}




LeapMotionClient::LeapMotionClient(Leap::Controller* leapController, vrinputemulator::VRInputEmulator* inputEmulator) 
		: leapController(leapController), inputEmulator(inputEmulator) {
	memset(&leftPose, 0, sizeof(vr::DriverPose_t));
	memset(&rightPose, 0, sizeof(vr::DriverPose_t));
	memset(&leftState, 0, sizeof(vr::VRControllerState_t));
	memset(&rightState, 0, sizeof(vr::VRControllerState_t));
	leftPose.qDriverFromHeadRotation = {1, 0, 0, 0};
	rightPose.qDriverFromHeadRotation = { 1, 0, 0, 0 };
}


LeapMotionClient::~LeapMotionClient() {
}

void LeapMotionClient::onFrame(const Leap::Controller& controller) {
	Leap::Frame frame0 = controller.frame();

	auto timeOffset = ((double)frame0.timestamp() - controller.now()) / 1000000.0;
	leftPose.poseTimeOffset = timeOffset;
	leftPose.deviceIsConnected = false;
	leftPose.poseIsValid = false;
	leftPose.result = vr::ETrackingResult::TrackingResult_Running_OutOfRange;

	rightPose.poseTimeOffset = timeOffset;
	rightPose.deviceIsConnected = false;
	rightPose.poseIsValid = false;
	rightPose.result = vr::ETrackingResult::TrackingResult_Running_OutOfRange;

	if (leftReadyFlag) {
		leftPose.deviceIsConnected = true;
	}
	if (rightReadyFlag) {
		rightPose.deviceIsConnected = true;
	}

	vr::TrackedDevicePose_t hmdPose;
	vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseRawAndUncalibrated, (float)timeOffset, &hmdPose, 1);
	if (hmdPose.bPoseIsValid) {
		bool leftPresent = false;
		bool rightPresent = false;

		leftPose.qWorldFromDriverRotation = leftRotationOffset * vrmath::quaternionFromRotationMatrix(hmdPose.mDeviceToAbsoluteTracking);
		leftPose.vecWorldFromDriverTranslation[0] = hmdPose.mDeviceToAbsoluteTracking.m[0][3] + leftTranslationOffset[0];
		leftPose.vecWorldFromDriverTranslation[1] = hmdPose.mDeviceToAbsoluteTracking.m[1][3] + leftTranslationOffset[1];
		leftPose.vecWorldFromDriverTranslation[2] = hmdPose.mDeviceToAbsoluteTracking.m[2][3] + leftTranslationOffset[2];

		rightPose.qWorldFromDriverRotation = rightRotationOffset * vrmath::quaternionFromRotationMatrix(hmdPose.mDeviceToAbsoluteTracking);
		rightPose.vecWorldFromDriverTranslation[0] = hmdPose.mDeviceToAbsoluteTracking.m[0][3] + rightTranslationOffset[0];
		rightPose.vecWorldFromDriverTranslation[1] = hmdPose.mDeviceToAbsoluteTracking.m[1][3] + rightTranslationOffset[1];
		rightPose.vecWorldFromDriverTranslation[2] = hmdPose.mDeviceToAbsoluteTracking.m[2][3] + rightTranslationOffset[2];

		auto handleHand = [this](const Leap::Hand& h, vr::DriverPose_t& pose, vr::VRControllerState_t& state, bool& handPresent, bool& readyFlag, const std::string& serial, uint32_t& virtualId) {
			double fingerBendFactor[5];
			Leap::Vector position;
			Leap::Vector velocity;
			Leap::Vector direction;
			for (auto& f : h.fingers()) {
				fingerBendFactor[(int)f.type()] = 0.0;
				Leap::Vector pd;
				for (int i = 0; i < 4; ++i) {
					auto& b = f.bone((Leap::Bone::Type)i);
					Leap::Vector d = -b.direction();
					if (i > 0) {
						auto a = d.angleTo(pd);
						fingerBendFactor[(int)f.type()] += a;
					}
					pd = d;
				}
				if (f.type() == Leap::Finger::TYPE_INDEX) {
					position = f.tipPosition();
					velocity = f.tipVelocity();
					direction = f.direction();
				}
				std::cout << f.type() << ": " << fingerBendFactor[(int)f.type()] << " - ";
			}

			handPresent = true;

			pose.vecPosition[0] = -0.001*position.x;
			pose.vecPosition[1] = -0.001*position.z;
			pose.vecPosition[2] = -0.001*position.y;

			pose.vecVelocity[0] = -0.001*velocity.x;
			pose.vecVelocity[1] = -0.001*velocity.z;
			pose.vecVelocity[2] = -0.001*velocity.y;

			Leap::Vector d = { -direction.x, -direction.z, -direction.y };
			Leap::Vector palmNormal = { -direction.x, -direction.z, -direction.y };
			float yaw = d.yaw();
			float pitch = d.pitch();
			float roll = palmNormal.roll();
			pose.qRotation = vrmath::quaternionFromYawPitchRoll(-yaw, pitch, roll);

			pose.poseIsValid = true;
			pose.result = vr::ETrackingResult::TrackingResult_Running_OK;

			if (!readyFlag) {
				virtualId = inputEmulator->addVirtualDevice(vrinputemulator::VirtualDeviceType::TrackedController, serial.c_str(), true);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_DeviceClass_Int32, (int32_t)vr::TrackedDeviceClass_Controller);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_SupportedButtons_Uint64, (uint64_t)
					vr::ButtonMaskFromId(vr::k_EButton_System) |
					vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu) |
					vr::ButtonMaskFromId(vr::k_EButton_Grip) |
					vr::ButtonMaskFromId(vr::k_EButton_Axis0) |
					vr::ButtonMaskFromId(vr::k_EButton_Axis1)
					);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_Axis0Type_Int32, (int32_t)vr::k_eControllerAxis_Joystick);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_Axis1Type_Int32, (int32_t)vr::k_eControllerAxis_Trigger);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_HardwareRevision_Uint64, (uint64_t)666);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_FirmwareVersion_Uint64, (uint64_t)666);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_RenderModelName_String, std::string("vr_controller_vive_1_5"));
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_ManufacturerName_String, std::string("Leap Motion"));
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_ModelNumber_String, std::string("Leap Motion Controller"));
				inputEmulator->publishVirtualDevice(virtualId);

				readyFlag = true;
			}

			if (fingerBendFactor[0] > 0.4) {
				state.rAxis[1] = { (float)std::min(1.0, std::max(0.0, (fingerBendFactor[0] - 0.4) / 0.6)), 0.0f };
				state.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_Axis1);
				if (state.rAxis[1].x >= 0.95) {
					state.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_Axis1);
				} else {
					state.ulButtonPressed &= ~vr::ButtonMaskFromId(vr::k_EButton_Axis1);
				}
			} else {
				state.rAxis[1] = { 0.0f, 0.0f };
				state.ulButtonPressed &= ~vr::ButtonMaskFromId(vr::k_EButton_Axis1);
				state.ulButtonTouched &= ~vr::ButtonMaskFromId(vr::k_EButton_Axis1);
			}

			inputEmulator->setVirtualDevicePose(virtualId, pose);
			inputEmulator->setVirtualControllerState(virtualId, state);
		};

		auto handleHand2 = [this](const Leap::Hand& h, vr::DriverPose_t& pose, vr::VRControllerState_t& state, bool& handPresent, bool& readyFlag, const std::string& serial, uint32_t& virtualId) {
			handPresent = true;

			Leap::Vector position = h.palmPosition();
			Leap::Vector velocity = h.palmVelocity();

			pose.vecPosition[0] = -0.001*position.x;
			pose.vecPosition[1] = -0.001*position.z;
			pose.vecPosition[2] = -0.001*position.y;

			pose.vecVelocity[0] = -0.001*velocity.x;
			pose.vecVelocity[1] = -0.001*velocity.z;
			pose.vecVelocity[2] = -0.001*velocity.y;

			Leap::Vector d = { -h.direction().x, -h.direction().z, -h.direction().y };
			Leap::Vector palmNormal = { -h.palmNormal().x, -h.palmNormal().z, -h.palmNormal().y };
			float yaw = d.yaw();
			float pitch = d.pitch();
			float roll = palmNormal.roll();
			pose.qRotation = vrmath::quaternionFromYawPitchRoll(-yaw, pitch, roll);

			pose.poseIsValid = true;
			pose.result = vr::ETrackingResult::TrackingResult_Running_OK;

			if (!readyFlag) {
				virtualId = inputEmulator->addVirtualDevice(vrinputemulator::VirtualDeviceType::TrackedController, serial.c_str(), true);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_DeviceClass_Int32, (int32_t)vr::TrackedDeviceClass_Controller);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_SupportedButtons_Uint64, (uint64_t)
					vr::ButtonMaskFromId(vr::k_EButton_System) |
					vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu) |
					vr::ButtonMaskFromId(vr::k_EButton_Grip) |
					vr::ButtonMaskFromId(vr::k_EButton_Axis0) |
					vr::ButtonMaskFromId(vr::k_EButton_Axis1)
				);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_Axis0Type_Int32, (int32_t)vr::k_eControllerAxis_Joystick);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_Axis1Type_Int32, (int32_t)vr::k_eControllerAxis_Trigger);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_HardwareRevision_Uint64, (uint64_t)666);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_FirmwareVersion_Uint64, (uint64_t)666);
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_RenderModelName_String, std::string("vr_controller_vive_1_5"));
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_ManufacturerName_String, std::string("Leap Motion"));
				inputEmulator->setVirtualDeviceProperty(virtualId, vr::Prop_ModelNumber_String, std::string("Leap Motion Controller"));
				inputEmulator->publishVirtualDevice(virtualId);

				readyFlag = true;
			}

			inputEmulator->setVirtualDevicePose(virtualId, pose);
			inputEmulator->setVirtualControllerState(virtualId, state);
		};

		for (auto& h : frame0.hands()) {
			if (h.isValid()) {

				if (h.isLeft()) {
					leftPresent = true;
					handleHand2(h, leftPose, leftState, leftPresent, leftReadyFlag, "leapmotion01", virtualIdLeft);

				} else if (h.isRight()) {
					rightPresent = true;
					handleHand2(h, rightPose, rightState, rightPresent, rightReadyFlag, "leapmotion02", virtualIdRight);
				}
			}
		}
	}

}
