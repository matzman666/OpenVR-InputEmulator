#pragma once

#include "common.h"
#include <openvr_driver.h>
#include <memory>
#include <map>


namespace vrinputemulator {
namespace driver {

// forward declarations
class ServerDriver;


class ITrackedDeviceServerDriver005Hooks : public InterfaceHooks {
public:
	typedef vr::EVRInitError (*activate_t)(void*, uint32_t unObjectId);

	static std::shared_ptr<InterfaceHooks> createHooks(void* iptr);
	virtual ~ITrackedDeviceServerDriver005Hooks();

private:
	HookData<activate_t> activateHook;
	void* activateAddress = nullptr;

	ITrackedDeviceServerDriver005Hooks(void* iptr);

	template<typename T>
	struct _hookedAdressMapEntry {
		unsigned useCount = 0;
		HookData<T> hookData;
	};
	static std::map<void*, _hookedAdressMapEntry<activate_t>> _hookedActivateAdressMap;

	static vr::EVRInitError _activate(void*, uint32_t unObjectId);

};

}
}


