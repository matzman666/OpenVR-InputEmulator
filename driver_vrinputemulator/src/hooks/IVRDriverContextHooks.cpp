#include "IVRDriverContextHooks.h"


namespace vrinputemulator {
namespace driver {


HookData<IVRDriverContextHooks::getGenericInterface_t> IVRDriverContextHooks::getGenericInterfaceHook;
std::map<std::string, std::shared_ptr<InterfaceHooks>> IVRDriverContextHooks::_hookedInterfaces;


IVRDriverContextHooks::IVRDriverContextHooks(void* iptr) {
	if (!_isHooked) {
		CREATE_MH_HOOK(getGenericInterfaceHook, _getGenericInterface, "IVRDriverContext::GetGenericInterface", iptr, 0);
		_isHooked = true;
	}
}


IVRDriverContextHooks::~IVRDriverContextHooks() {
	if (_isHooked) {
		REMOVE_MH_HOOK(getGenericInterfaceHook);
		_isHooked = false;
	}
}


std::shared_ptr<InterfaceHooks> IVRDriverContextHooks::createHooks(void * iptr) {
	std::shared_ptr<InterfaceHooks> retval = std::shared_ptr<InterfaceHooks>(new IVRDriverContextHooks(iptr));
	return retval;
}

std::shared_ptr<InterfaceHooks> IVRDriverContextHooks::getInterfaceHook(std::string interfaceVersion) {
	auto it = _hookedInterfaces.find(interfaceVersion);
	if (it != _hookedInterfaces.end()){
		return it->second;
	}
	return nullptr;
}


void* IVRDriverContextHooks::_getGenericInterface(vr::IVRDriverContext* _this, const char *pchInterfaceVersion, vr::EVRInitError *peError) {
	auto retval = getGenericInterfaceHook.origFunc(_this, pchInterfaceVersion, peError);
	if (_hookedInterfaces.find(pchInterfaceVersion) == _hookedInterfaces.end()) {
		auto hooks = InterfaceHooks::hookInterface(retval, pchInterfaceVersion);
		if (hooks != nullptr) {
			_hookedInterfaces.insert({ std::string(pchInterfaceVersion), hooks });
		}
	}
	LOG(TRACE) << "IVRDriverContextHooks::_getGenericInterface(" << _this << ", " << pchInterfaceVersion << ") = " << retval;
	return retval;
}


}
}
