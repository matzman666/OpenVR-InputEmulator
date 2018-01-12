#pragma once

#include "common.h"
#include <openvr_driver.h>
#include <memory>
#include <map>


namespace vrinputemulator {
namespace driver {

// forward declarations
class ServerDriver;


class IVRDriverContextHooks : public InterfaceHooks {
public:
	typedef void*(*getGenericInterface_t)(vr::IVRDriverContext*, const char *pchInterfaceVersion, vr::EVRInitError *peError);

	static std::shared_ptr<InterfaceHooks> getInterfaceHook(std::string interfaceVersion);

	static std::shared_ptr<InterfaceHooks> createHooks(void* iptr);
	virtual ~IVRDriverContextHooks();

private:
	bool _isHooked = false;

	IVRDriverContextHooks(void* iptr);

	static HookData<getGenericInterface_t> getGenericInterfaceHook;

	static std::map<std::string, std::shared_ptr<InterfaceHooks>> _hookedInterfaces;
	static void* _getGenericInterface(vr::IVRDriverContext*, const char *pchInterfaceVersion, vr::EVRInitError *peError);

};

}
}


