#pragma once

#include <stdint.h>


namespace vrinputemulator {

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

} // end namespace vrinputemulator
