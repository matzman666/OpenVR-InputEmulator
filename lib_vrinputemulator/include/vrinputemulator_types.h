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
		bool buttonMappingEnabled;
		bool redirectSuspended;
	};

	enum class MotionCompensationVelAccMode : uint32_t {
		Disabled = 0,
		SetZero = 1,
		SubstractMotionRef = 2,
		LinearApproximation = 3,
		KalmanFilter = 4
	};

} // end namespace vrinputemulator
