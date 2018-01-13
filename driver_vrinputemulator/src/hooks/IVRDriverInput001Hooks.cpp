#include "IVRDriverInput001Hooks.h"

#include "../driver/ServerDriver.h"


namespace vrinputemulator {
namespace driver {


HookData<IVRDriverInput001Hooks::createBooleanComponent_t> IVRDriverInput001Hooks::createBooleanComponentHook;
HookData<IVRDriverInput001Hooks::updateBooleanComponent_t> IVRDriverInput001Hooks::updateBooleanComponentHook;
HookData<IVRDriverInput001Hooks::createScalarComponent_t> IVRDriverInput001Hooks::createScalarComponentHook;
HookData<IVRDriverInput001Hooks::updateScalarComponent_t> IVRDriverInput001Hooks::updateScalarComponentHook;
HookData<IVRDriverInput001Hooks::createHapticComponent_t> IVRDriverInput001Hooks::createHapticComponentHook;


IVRDriverInput001Hooks::IVRDriverInput001Hooks(void* iptr) {
	if (!_isHooked) {
		CREATE_MH_HOOK(createBooleanComponentHook, _createBooleanComponent, "IVRDriverInput001Hooks::CreateBooleanComponent", iptr, 0);
		CREATE_MH_HOOK(updateBooleanComponentHook, _updateBooleanComponent, "IVRDriverInput001Hooks::UpdateBooleanComponent", iptr, 1);
		CREATE_MH_HOOK(createScalarComponentHook, _createScalarComponent, "IVRDriverInput001Hooks::CreateScalarComponent", iptr, 2);
		CREATE_MH_HOOK(updateScalarComponentHook, _updateScalarComponent, "IVRDriverInput001Hooks::UpdateScalarComponent", iptr, 3);
		CREATE_MH_HOOK(createHapticComponentHook, _createHapticComponent, "IVRDriverInput001Hooks::CreateHapticComponent", iptr, 4);
		_isHooked = true;
	}
}


IVRDriverInput001Hooks::~IVRDriverInput001Hooks() {
	if (_isHooked) {
		REMOVE_MH_HOOK(createBooleanComponentHook);
		REMOVE_MH_HOOK(updateBooleanComponentHook);
		REMOVE_MH_HOOK(createScalarComponentHook);
		REMOVE_MH_HOOK(updateScalarComponentHook);
		REMOVE_MH_HOOK(createHapticComponentHook);
		_isHooked = false;
	}
}


std::shared_ptr<InterfaceHooks> IVRDriverInput001Hooks::createHooks(void * iptr) {
	std::shared_ptr<InterfaceHooks> retval = std::shared_ptr<InterfaceHooks>(new IVRDriverInput001Hooks(iptr));
	return retval;
}

vr::EVRInputError IVRDriverInput001Hooks::updateBooleanComponentOrig(void* _this, vr::VRInputComponentHandle_t ulComponent, bool bNewValue, double fTimeOffset) {
	return updateBooleanComponentHook.origFunc(_this, ulComponent, bNewValue, fTimeOffset);
}

vr::EVRInputError IVRDriverInput001Hooks::updateScalarComponentOrig(void* _this, vr::VRInputComponentHandle_t ulComponent, float fNewValue, double fTimeOffset) {
	return updateScalarComponentHook.origFunc(_this, ulComponent, fNewValue, fTimeOffset);
}


vr::EVRInputError IVRDriverInput001Hooks::_createBooleanComponent(void* _this, vr::PropertyContainerHandle_t ulContainer, const char *pchName, void* pHandle) {
	auto retval = createBooleanComponentHook.origFunc(_this, ulContainer, pchName, pHandle);
	if (retval == 0) {
		serverDriver->hooksCreateBooleanComponent(_this, 1, ulContainer, pchName, pHandle);
	}
	LOG(TRACE) << "IVRDriverInput001Hooks::_createBooleanComponent(" << _this << ", " << ulContainer << ", " << pchName << ", " << pHandle << ") = " << (int)retval;
	return retval;
}

vr::EVRInputError IVRDriverInput001Hooks::_updateBooleanComponent(void* _this, vr::VRInputComponentHandle_t ulComponent, bool bNewValue, double fTimeOffset) {
	LOG(TRACE) << "IVRDriverInput001Hooks::_updateBooleanComponent(" << _this << ", " << ulComponent << ", " << bNewValue << ", " << fTimeOffset << ")";
	if (serverDriver->hooksUpdateBooleanComponent(_this, 1, ulComponent, bNewValue, fTimeOffset)) {
		return updateBooleanComponentHook.origFunc(_this, ulComponent, bNewValue, fTimeOffset);
	}
	return (vr::EVRInputError)0;
}

vr::EVRInputError IVRDriverInput001Hooks::_createScalarComponent(void* _this, vr::PropertyContainerHandle_t ulContainer, const char *pchName, void* pHandle, vr::EVRScalarType eType, vr::EVRScalarUnits eUnits) {
	auto retval = createScalarComponentHook.origFunc(_this, ulContainer, pchName, pHandle, eType, eUnits);
	if (retval == 0) {
		serverDriver->hooksCreateScalarComponent(_this, 1, ulContainer, pchName, pHandle, eType, eUnits);
	}
	LOG(TRACE) << "IVRDriverInput001Hooks::_createScalarComponent(" << _this << ", " << ulContainer << ", " << pchName << ", " << pHandle << ", " << (int)eType << ", " << (int)eUnits << ") = " << (int)retval;
	return retval;
}

vr::EVRInputError IVRDriverInput001Hooks::_updateScalarComponent(void* _this, vr::VRInputComponentHandle_t ulComponent, float fNewValue, double fTimeOffset) {
	LOG(TRACE) << "IVRDriverInput001Hooks::_updateScalarComponent(" << _this << ", " << ulComponent << ", " << fNewValue << ", " << fTimeOffset << ")";
	if (serverDriver->hooksUpdateScalarComponent(_this, 1, ulComponent, fNewValue, fTimeOffset)) {
		return updateScalarComponentHook.origFunc(_this, ulComponent, fNewValue, fTimeOffset);
	}
	return (vr::EVRInputError)0;
}

vr::EVRInputError IVRDriverInput001Hooks::_createHapticComponent(void* _this, vr::PropertyContainerHandle_t ulContainer, const char *pchName, void* pHandle) {
	auto retval = createHapticComponentHook.origFunc(_this, ulContainer, pchName, pHandle);
	if (retval == 0) {
		serverDriver->hooksCreateHapticComponent(_this, 1, ulContainer, pchName, pHandle);
	}
	LOG(TRACE) << "IVRDriverInput001Hooks::_createHapticComponent(" << _this << ", " << ulContainer << ", " << pchName << ", " << pHandle << ") = " << (int)retval;
	return retval;
}


}
}
