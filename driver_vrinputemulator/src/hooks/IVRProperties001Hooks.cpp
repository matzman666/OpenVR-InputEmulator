#include "IVRProperties001Hooks.h"

#include "../driver/ServerDriver.h"


namespace vrinputemulator {
namespace driver {


HookData<IVRProperties001Hooks::readPropertyBatch_t> IVRProperties001Hooks::readPropertyBatchHook;
HookData<IVRProperties001Hooks::writePropertyBatch_t> IVRProperties001Hooks::writePropertyBatchHook;


IVRProperties001Hooks::IVRProperties001Hooks(void* iptr) {
	if (!_isHooked) {
		CREATE_MH_HOOK(readPropertyBatchHook, _readPropertyBatch, "IVRproperties001Hooks::ReadPropertyBatch", iptr, 0);
		CREATE_MH_HOOK(writePropertyBatchHook, _writePropertyBatch, "IVRproperties001Hooks::WritePropertyBatch", iptr, 1);
		_isHooked = true;
	}
}


IVRProperties001Hooks::~IVRProperties001Hooks() {
	if (_isHooked) {
		REMOVE_MH_HOOK(readPropertyBatchHook);
		REMOVE_MH_HOOK(writePropertyBatchHook);
		_isHooked = false;
	}
}


std::shared_ptr<InterfaceHooks> IVRProperties001Hooks::createHooks(void * iptr) {
	std::shared_ptr<InterfaceHooks> retval = std::shared_ptr<InterfaceHooks>(new IVRProperties001Hooks(iptr));
	return retval;
}


vr::ETrackedPropertyError IVRProperties001Hooks::_readPropertyBatch(void* _this, vr::PropertyContainerHandle_t ulContainer, void* pBatch, uint32_t unBatchEntryCount) {
	auto retval = readPropertyBatchHook.origFunc(_this, ulContainer, pBatch, unBatchEntryCount);
	serverDriver->hooksPropertiesReadPropertyBatch(_this, 1, ulContainer, pBatch, unBatchEntryCount);
	//LOG(TRACE) << "IVRproperties001Hooks::_readPropertyBatch(" << _this << ", " << ulContainer << ", " << pBatch << ", " << unBatchEntryCount << ") = " << (int)retval;
	return retval;
}

vr::ETrackedPropertyError IVRProperties001Hooks::_writePropertyBatch(void* _this, vr::PropertyContainerHandle_t ulContainer, void* pBatch, uint32_t unBatchEntryCount) {
	serverDriver->hooksPropertiesWritePropertyBatch(_this, 1, ulContainer, pBatch, unBatchEntryCount);
	auto retval = writePropertyBatchHook.origFunc(_this, ulContainer, pBatch, unBatchEntryCount);
	//LOG(TRACE) << "IVRproperties001Hooks::_writePropertyBatch(" << _this << ", " << ulContainer << ", " << pBatch << ", " << unBatchEntryCount << ") = " << (int)retval;
	return retval;
}


}
}
