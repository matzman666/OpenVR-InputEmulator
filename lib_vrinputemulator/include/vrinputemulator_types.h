#pragma once

#include <stdint.h>


namespace vrinputemulator {

	static const char* const vrsettings_SectionName = "driver_00vrinputemulator";
	static const char* const vrsettings_overrideHmdManufacturer_string = "overrideHmdManufacturer";
	static const char* const vrsettings_overrideHmdModel_string = "overrideHmdModel";
	static const char* const vrsettings_overrideHmdTrackingSystem_string = "overrideHmdTrackingSystem";
	static const char* const vrsettings_genericTrackerFakeController_bool = "genericTrackerFakeController";

	enum class VirtualDeviceType : uint32_t {
		None = 0,
		TrackedController = 1
	};


	enum class ButtonEventType : uint32_t {
		None = 0,
		ButtonPressed = 1,
		ButtonUnpressed = 2,
		ButtonTouched = 3,
		ButtonUntouched = 4,
	};


	enum class DevicePropertyValueType : uint32_t {
		None = 0,
		FLOAT = 1,
		INT32 = 2,
		UINT64 = 3,
		BOOL = 4,
		STRING = 5,
		MATRIX34 = 20,
		MATRIX44 = 21,
		VECTOR3 = 22,
		VECTOR4 = 23
	};


	struct DeviceOffsets {
		uint32_t deviceId;
		bool offsetsEnabled;
		vr::HmdQuaternion_t worldFromDriverRotationOffset;
		vr::HmdVector3d_t worldFromDriverTranslationOffset;
		vr::HmdQuaternion_t driverFromHeadRotationOffset;
		vr::HmdVector3d_t driverFromHeadTranslationOffset;
		vr::HmdQuaternion_t deviceRotationOffset;
		vr::HmdVector3d_t deviceTranslationOffset;
	};


	struct DeviceInfo {
		uint32_t deviceId;
		vr::ETrackedDeviceClass deviceClass;
		int deviceMode;
		bool offsetsEnabled;
		bool redirectSuspended;
	};


	enum class MotionCompensationVelAccMode : uint32_t {
		Disabled = 0,
		SetZero = 1,
		SubstractMotionRef = 2,
		LinearApproximation = 3,
		KalmanFilter = 4
	};


	enum class DigitalBindingType : uint32_t {
		NoRemapping = 0,
		Disabled = 1,
		OpenVR = 2,
		Keyboard = 3,
		SuspendRedirectMode = 4
	};


	struct DigitalBinding {
		DigitalBindingType type = DigitalBindingType::NoRemapping;
		
		union BindingUnion {
			struct {
				uint32_t controllerId;
				uint32_t buttonId;
			} openvr;

			struct {
				bool shiftPressed = false;
				bool ctrlPressed = false;
				bool altPressed = false;
				uint32_t keyCode = 0x00;
			} keyboard;

			BindingUnion() {}
		} binding;
		
		bool toggleEnabled;
		uint32_t toggleDelay;
		
		bool autoTriggerEnabled;
		uint32_t autoTriggerFrequency;

		DigitalBinding() {}
	};


	struct DigitalInputRemapping {
		bool valid;
		DigitalBinding binding;

		bool touchAsClick = false;

		bool longPressEnabled = false;
		uint32_t longPressThreshold = 1000;
		DigitalBinding longPressBinding;

		bool doublePressEnabled = false;
		uint32_t doublePressThreshold = 300;
		DigitalBinding doublePressBinding;

		DigitalInputRemapping(bool valid = false) : valid(valid) {}
	};


	enum class AnalogBindingType : uint32_t {
		NoRemapping,
		Disabled,
		OpenVR
	};


	struct AnalogBinding {
		AnalogBindingType type;

		union {
			struct {
				uint32_t controllerId;
				uint32_t axisId;
				uint32_t axisDimId;
			} openvr;
		} binding;
		bool invertAxis;
		float lowerDeadzone = 0.0;
		float upperDeadzone = 1.0;
		bool autoTriggerEnabled;
		uint32_t autoTriggerFrequency;
	};


	enum class AnalogInputRemappingMode : uint32_t {
		Normal,
		ButtonField
	};


	enum class AnalogInputButtonPattern : uint32_t {
		DPAD
	};


	struct AnalogInputRemapping {
		AnalogInputRemappingMode mode;

		union {
			struct {
				AnalogBinding bindings[2];
			} normalMode;

			struct {
				AnalogInputButtonPattern pattern;
				float deadzone;
			} buttonMode;
		} data;
	};


} // end namespace vrinputemulator
