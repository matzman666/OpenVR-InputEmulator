#pragma once

#include "common.h"
#include <memory>
#include <openvr_driver.h>


namespace vrinputemulator {
namespace driver {


class IVRDriverInput001Hooks : public InterfaceHooks {
public:
	typedef vr::EVRInputError(*createBooleanComponent_t)(void*, vr::PropertyContainerHandle_t, const char*, void*);
	typedef vr::EVRInputError(*updateBooleanComponent_t)(void*, vr::VRInputComponentHandle_t, bool, double);
	typedef vr::EVRInputError(*createScalarComponent_t)(void*, vr::PropertyContainerHandle_t, const char*, void*, vr::EVRScalarType, vr::EVRScalarUnits);
	typedef vr::EVRInputError(*updateScalarComponent_t)(void*, vr::VRInputComponentHandle_t, float, double);
	typedef vr::EVRInputError(*createHapticComponent_t)(void*, vr::PropertyContainerHandle_t, const char*, void*);

	static std::shared_ptr<InterfaceHooks> createHooks(void* iptr);
	virtual ~IVRDriverInput001Hooks();

	static vr::EVRInputError updateBooleanComponentOrig(void* _this, vr::VRInputComponentHandle_t ulComponent, bool bNewValue, double fTimeOffset);
	static vr::EVRInputError updateScalarComponentOrig(void* _this, vr::VRInputComponentHandle_t ulComponent, float fNewValue, double fTimeOffset);

private:
	bool _isHooked = false;

	IVRDriverInput001Hooks(void* iptr);

	static HookData<createBooleanComponent_t> createBooleanComponentHook;
	static HookData<updateBooleanComponent_t> updateBooleanComponentHook;
	static HookData<createScalarComponent_t> createScalarComponentHook;
	static HookData<updateScalarComponent_t> updateScalarComponentHook;
	static HookData<createHapticComponent_t> createHapticComponentHook;

	static vr::EVRInputError _createBooleanComponent(void* _this, vr::PropertyContainerHandle_t ulContainer, const char *pchName, void* pHandle);
	static vr::EVRInputError _updateBooleanComponent(void* _this, vr::VRInputComponentHandle_t ulComponent, bool bNewValue, double fTimeOffset);
	static vr::EVRInputError _createScalarComponent(void* _this, vr::PropertyContainerHandle_t ulContainer, const char *pchName, void* pHandle, vr::EVRScalarType eType, vr::EVRScalarUnits eUnits);
	static vr::EVRInputError _updateScalarComponent(void* _this, vr::VRInputComponentHandle_t ulComponent, float fNewValue, double fTimeOffset);
	static vr::EVRInputError _createHapticComponent(void* _this, vr::PropertyContainerHandle_t ulContainer, const char *pchName, void* pHandle);

};

}
}
