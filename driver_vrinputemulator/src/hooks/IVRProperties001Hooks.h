#pragma once

#include "common.h"
#include <memory>
#include <openvr_driver.h>


namespace vrinputemulator {
namespace driver {


class IVRProperties001Hooks : public InterfaceHooks {
public:
	typedef vr::ETrackedPropertyError(*readPropertyBatch_t)(void*, vr::PropertyContainerHandle_t, void*, uint32_t);
	typedef vr::ETrackedPropertyError(*writePropertyBatch_t)(void*, vr::PropertyContainerHandle_t, void*, uint32_t);

	static std::shared_ptr<InterfaceHooks> createHooks(void* iptr);
	virtual ~IVRProperties001Hooks();

private:
	bool _isHooked = false;

	IVRProperties001Hooks(void* iptr);

	static HookData<readPropertyBatch_t> readPropertyBatchHook;
	static HookData<writePropertyBatch_t> writePropertyBatchHook;

	static vr::ETrackedPropertyError _readPropertyBatch(void* _this, vr::PropertyContainerHandle_t ulContainer, void* pBatch, uint32_t unBatchEntryCount);
	static vr::ETrackedPropertyError _writePropertyBatch(void* _this, vr::PropertyContainerHandle_t ulContainer, void* pBatch, uint32_t unBatchEntryCount);

};

}
}
