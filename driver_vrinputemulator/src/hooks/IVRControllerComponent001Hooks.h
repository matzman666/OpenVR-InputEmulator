#pragma once

#include "common.h"
#include <openvr_driver.h>
#include <memory>
#include <map>


namespace vrinputemulator {
namespace driver {

// forward declarations
class ServerDriver;


class IVRControllerComponent001Hooks : public InterfaceHooks {
public:
	typedef bool(*triggerHapticPulse_t)(void*, uint32_t unAxisId, uint16_t usPulseDurationMicroseconds);

	static std::shared_ptr<InterfaceHooks> createHooks(void* iptr);
	virtual ~IVRControllerComponent001Hooks();

	bool triggerHapticPulseOrig(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds);

private:
	HookData<triggerHapticPulse_t> triggerHapicPulseHook;
	void* triggerHapicPulseAddress;
	void* controllerComponent;

	IVRControllerComponent001Hooks(void* iptr);

	template<typename T>
	struct _hookedAdressMapEntry {
		unsigned useCount = 0;
		HookData<T> hookData;
	};
	static std::map<void*, _hookedAdressMapEntry<triggerHapticPulse_t>> _hookedTriggerHapticPulseAdressMap;

	static bool _triggerHapticPulse(void* _this, uint32_t unAxisId, uint16_t usPulseDurationMicroseconds);

};


}
}

