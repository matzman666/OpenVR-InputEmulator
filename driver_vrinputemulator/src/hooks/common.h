#pragma once

#include <string>
#include <stdint.h>
#include <MinHook.h>
#include "../logging.h"


namespace vr {
	enum EVRInputError;
	enum EVRScalarType;
	enum EVRScalarUnits;
	typedef uint64_t VRInputComponentHandle_t;
}


namespace vrinputemulator {
namespace driver {


#define CREATE_MH_HOOK(detourInfo, detourFunc, logName, objPtr, vtableOffset) {\
	detourInfo.targetFunc = (*((void***)objPtr))[vtableOffset]; \
	MH_STATUS mhError = MH_CreateHook(detourInfo.targetFunc, (void*)&detourFunc, reinterpret_cast<LPVOID*>(&detourInfo.origFunc)); \
	if (mhError == MH_OK) { \
		mhError = MH_EnableHook(detourInfo.targetFunc); \
		if (mhError == MH_OK) { \
			detourInfo.enabled = true; \
			LOG(INFO) << logName << " hook is enabled (Address: " << std::hex << detourInfo.targetFunc << std::dec << ")"; \
		} else { \
			MH_RemoveHook(detourInfo.targetFunc); \
			LOG(ERROR) << "Error while enabling " << logName << " hook: " << MH_StatusToString(mhError); \
		} \
	} else { \
		LOG(ERROR) << "Error while creating " << logName << " hook: " << MH_StatusToString(mhError); \
	}\
}


#define REMOVE_MH_HOOK(detourInfo) {\
	if (detourInfo.enabled) { \
		MH_RemoveHook(detourInfo.targetFunc); \
		detourInfo.enabled = false; \
	}\
}


//forward declarations
class ServerDriver;
class IVRDriverContextHooks;
class IVRServerDriverHost004Hooks;

template<class T>
struct HookData {
	bool enabled = false;
	void* targetFunc = nullptr;
	T origFunc = nullptr;
};



class InterfaceHooks {
public:
	virtual ~InterfaceHooks() {}

	static std::shared_ptr<InterfaceHooks> hookInterface(void* interfaceRef, std::string interfaceVersion);

	static void setServerDriver(ServerDriver* driver) { serverDriver = driver; }

protected:
	static ServerDriver* serverDriver;
};


}
}
