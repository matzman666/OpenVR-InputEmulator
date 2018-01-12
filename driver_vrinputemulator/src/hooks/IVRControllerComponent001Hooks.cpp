#include "IVRControllerComponent001Hooks.h"

#include "../driver/ServerDriver.h"


namespace vrinputemulator {
namespace driver {


std::map<void*, IVRControllerComponent001Hooks::_hookedAdressMapEntry<IVRControllerComponent001Hooks::triggerHapticPulse_t>> IVRControllerComponent001Hooks::_hookedTriggerHapticPulseAdressMap;


IVRControllerComponent001Hooks::IVRControllerComponent001Hooks(void* iptr) : controllerComponent(iptr) {
	LOG(TRACE) << "IVRControllerComponent001Hooks::ctr(" << iptr << ")";
	auto vtable = (*((void***)iptr));
	triggerHapicPulseAddress = vtable[1];
	auto it = _hookedTriggerHapticPulseAdressMap.find(triggerHapicPulseAddress);
	if (it == _hookedTriggerHapticPulseAdressMap.end()) {
		CREATE_MH_HOOK(triggerHapicPulseHook, _triggerHapticPulse, "IVRControllerComponent001Hooks::TriggerHapticPulse", iptr, 1);
		_hookedTriggerHapticPulseAdressMap[triggerHapicPulseAddress].useCount = 1;
		_hookedTriggerHapticPulseAdressMap[triggerHapicPulseAddress].hookData = triggerHapicPulseHook;
	} else {
		triggerHapicPulseHook = it->second.hookData;
		it->second.useCount += 1;
	}
}


std::shared_ptr<InterfaceHooks> IVRControllerComponent001Hooks::createHooks(void * iptr) {
	std::shared_ptr<InterfaceHooks> retval = std::shared_ptr<InterfaceHooks>(new IVRControllerComponent001Hooks(iptr));
	return retval;
}


IVRControllerComponent001Hooks::~IVRControllerComponent001Hooks() {
	auto it = _hookedTriggerHapticPulseAdressMap.find(triggerHapicPulseAddress);
	if (it != _hookedTriggerHapticPulseAdressMap.end()) {
		if (it->second.useCount <= 1) {
			REMOVE_MH_HOOK(triggerHapicPulseHook);
			_hookedTriggerHapticPulseAdressMap.erase(it);
		} else {
			it->second.useCount -= 1;
		}
	}
}

bool IVRControllerComponent001Hooks::triggerHapticPulseOrig(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds) {
	return triggerHapicPulseHook.origFunc(controllerComponent, unAxisId, usPulseDurationMicroseconds);
}


bool IVRControllerComponent001Hooks::_triggerHapticPulse(void* _this, uint32_t unAxisId, uint16_t usPulseDurationMicroseconds) {
	LOG(TRACE) << "IVRControllerComponent001Hooks::_triggerHapticPulse(" << _this << ", " << unAxisId << ", " << usPulseDurationMicroseconds << ")";
	if (serverDriver->hooksControllerTriggerHapticPulse(_this, 1, unAxisId, usPulseDurationMicroseconds)) {
		auto vtable = (*((void***)_this));
		auto triggerHapticAddress = vtable[1];
		auto it = _hookedTriggerHapticPulseAdressMap.find(triggerHapticAddress);
		if (it != _hookedTriggerHapticPulseAdressMap.end()) {
			return it->second.hookData.origFunc(_this, unAxisId, usPulseDurationMicroseconds);
		} else {
			LOG(ERROR) << "this pointer not in IVRControllerComponent001Hooks::_hookedTriggerHapticPulseAdressMap.";
		}
	}
	return true;
}


}
}
