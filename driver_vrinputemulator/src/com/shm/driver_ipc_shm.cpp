#include "driver_ipc_shm.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <openvr_driver.h>
#include <ipc_protocol.h>
#include <openvr_math.h>
#include "../../driver/ServerDriver.h"
#include "../../driver/VirtualDeviceDriver.h"
#include "../../devicemanipulation/DeviceManipulationHandle.h"


namespace vrinputemulator {
namespace driver {


void IpcShmCommunicator::init(ServerDriver* driver) {
	_driver = driver;
	_ipcThreadStopFlag = false;
	_ipcThread = std::thread(_ipcThreadFunc, this, driver);
}

void IpcShmCommunicator::shutdown() {
	if (_ipcThreadRunning) {
		_ipcThreadStopFlag = true;
		_ipcThread.join();
	}
}

void IpcShmCommunicator::sendReplySetMotionCompensationMode(bool success) {
	if (_setMotionCompensationMessageId != 0) {
		ipc::Reply resp(ipc::ReplyType::GenericReply);
		resp.messageId = _setMotionCompensationMessageId;
		if (success) {
			resp.status = ipc::ReplyStatus::Ok;
		} else {
			resp.status = ipc::ReplyStatus::NotTracking;
		}
		sendReply(_setMotionCompensationClientId, resp);
	}
	_setMotionCompensationMessageId = 0;
}

void IpcShmCommunicator::_ipcThreadFunc(IpcShmCommunicator* _this, ServerDriver * driver) {
	_this->_ipcThreadRunning = true;
	LOG(DEBUG) << "CServerDriver::_ipcThreadFunc: thread started";
	try {
		// Create message queue
		boost::interprocess::message_queue::remove(_this->_ipcQueueName.c_str());
		boost::interprocess::message_queue messageQueue(
			boost::interprocess::create_only,
			_this->_ipcQueueName.c_str(),
			100,					//max message number
			sizeof(ipc::Request)    //max message size
			);

		while (!_this->_ipcThreadStopFlag) {
			try {
				ipc::Request message;
				uint64_t recv_size;
				unsigned priority;
				boost::posix_time::ptime timeout = boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(50);
				if (messageQueue.timed_receive(&message, sizeof(ipc::Request), recv_size, priority, timeout)) {
					LOG(TRACE) << "CServerDriver::_ipcThreadFunc: IPC request received ( type " << (int)message.type << ")";
					if (recv_size == sizeof(ipc::Request)) {
						switch (message.type) {

						case ipc::RequestType::IPC_ClientConnect:
							{
								try {
									auto queue = std::make_shared<boost::interprocess::message_queue>(boost::interprocess::open_only, message.msg.ipc_ClientConnect.queueName);
									ipc::Reply reply(ipc::ReplyType::IPC_ClientConnect);
									reply.messageId = message.msg.ipc_ClientConnect.messageId;
									reply.msg.ipc_ClientConnect.ipcProcotolVersion = IPC_PROTOCOL_VERSION;
									uint32_t clientId = 0;
									if (message.msg.ipc_ClientConnect.ipcProcotolVersion == IPC_PROTOCOL_VERSION) {
										clientId = _this->_ipcClientIdNext++;
										_this->_ipcEndpoints.insert({ clientId, queue });
										reply.msg.ipc_ClientConnect.clientId = clientId;
										reply.status = ipc::ReplyStatus::Ok;
										LOG(INFO) << "New client connected: endpoint \"" << message.msg.ipc_ClientConnect.queueName << "\", cliendId " << clientId;
									} else {
										reply.msg.ipc_ClientConnect.clientId = 0;
										reply.status = ipc::ReplyStatus::InvalidVersion;
										LOG(INFO) << "Client (endpoint \"" << message.msg.ipc_ClientConnect.queueName << "\") reports incompatible ipc version "
											<< message.msg.ipc_ClientConnect.ipcProcotolVersion;
									}
									_this->sendReply(clientId, reply);
								} catch (std::exception& e) {
									LOG(ERROR) << "Error during client connect: " << e.what();
								}
							}
							break;

						case ipc::RequestType::IPC_ClientDisconnect:
							{
								ipc::Reply reply(ipc::ReplyType::GenericReply);
								reply.messageId = message.msg.ipc_ClientDisconnect.messageId;
								auto i = _this->_ipcEndpoints.find(message.msg.ipc_ClientDisconnect.clientId);
								if (i != _this->_ipcEndpoints.end()) {
									reply.status = ipc::ReplyStatus::Ok;
									LOG(INFO) << "Client disconnected: clientId " << message.msg.ipc_ClientDisconnect.clientId;
									if (reply.messageId != 0) {
										_this->sendReply(message.msg.ipc_ClientDisconnect.clientId, reply);
									}
									_this->_ipcEndpoints.erase(i);
								} else {
									LOG(ERROR) << "Error during client disconnect: unknown clientID " << message.msg.ipc_ClientDisconnect.clientId;
								}
							}
							break;

						case ipc::RequestType::IPC_Ping:
							{
								LOG(TRACE) << "Ping received: clientId " << message.msg.ipc_Ping.clientId << ", nonce " << message.msg.ipc_Ping.nonce;
								ipc::Reply reply(ipc::ReplyType::IPC_Ping);
								reply.messageId = message.msg.ipc_Ping.messageId;
								reply.status = ipc::ReplyStatus::Ok;
								reply.msg.ipc_Ping.nonce = message.msg.ipc_Ping.nonce;
								_this->sendReply(message.msg.ipc_ClientDisconnect.clientId, reply);
							}
							break;

						case ipc::RequestType::OpenVR_ButtonEvent:
							{
								if (vr::VRServerDriverHost()) {
									unsigned iterCount = min(message.msg.ipc_ButtonEvent.eventCount, REQUEST_OPENVR_BUTTONEVENT_MAXCOUNT);
									for (unsigned i = 0; i < iterCount; ++i) {
										auto& e = message.msg.ipc_ButtonEvent.events[i];
										try {
											driver->openvr_buttonEvent(e.deviceId, e.eventType, e.buttonId, e.timeOffset);
										} catch (std::exception& e) {
											LOG(ERROR) << "Error in ipc thread: " << e.what();
										}
									}
								}
							}
							break;

						case ipc::RequestType::OpenVR_AxisEvent:
							{
								if (vr::VRServerDriverHost()) {
									for (unsigned i = 0; i < message.msg.ipc_AxisEvent.eventCount; ++i) {
										auto& e = message.msg.ipc_AxisEvent.events[i];
										driver->openvr_axisEvent(e.deviceId, e.axisId, e.axisState);
									}
								}
							}
							break;

						case ipc::RequestType::OpenVR_PoseUpdate:
							{
								if (vr::VRServerDriverHost()) {
									driver->openvr_poseUpdate(message.msg.ipc_PoseUpdate.deviceId, message.msg.ipc_PoseUpdate.pose, message.timestamp);
								}
							}
							break;

						case ipc::RequestType::OpenVR_ProximitySensorEvent:
							{
								driver->openvr_proximityEvent(message.msg.ipc_PoseUpdate.deviceId, message.msg.ovr_ProximitySensorEvent.sensorTriggered);
							}
							break;

						case ipc::RequestType::OpenVR_VendorSpecificEvent:
							{
								driver->openvr_vendorSpecificEvent(message.msg.ovr_VendorSpecificEvent.deviceId, message.msg.ovr_VendorSpecificEvent.eventType,
									message.msg.ovr_VendorSpecificEvent.eventData, message.msg.ovr_VendorSpecificEvent.timeOffset);
							}
							break;

						case ipc::RequestType::VirtualDevices_GetDeviceCount:
							{
								ipc::Reply reply(ipc::ReplyType::VirtualDevices_GetDeviceCount);
								reply.messageId = message.msg.vd_GenericClientMessage.messageId;
								reply.status = ipc::ReplyStatus::Ok;
								reply.msg.vd_GetDeviceCount.deviceCount = driver->virtualDevices_getDeviceCount();
								_this->sendReply(message.msg.vd_GenericClientMessage.clientId, reply);
							}
							break;

						case ipc::RequestType::VirtualDevices_GetDeviceInfo:
							{
								ipc::Reply resp(ipc::ReplyType::VirtualDevices_GetDeviceInfo);
								resp.messageId = message.msg.vd_GenericDeviceIdMessage.messageId;
								if (message.msg.vd_GenericDeviceIdMessage.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									auto d = driver->virtualDevices_getDevice(message.msg.vd_GenericDeviceIdMessage.deviceId);
									if (!d) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										resp.msg.vd_GetDeviceInfo.virtualDeviceId = message.msg.vd_GenericDeviceIdMessage.deviceId;
										resp.msg.vd_GetDeviceInfo.openvrDeviceId = d->openvrDeviceId();
										resp.msg.vd_GetDeviceInfo.deviceType = d->deviceType();
										strncpy_s(resp.msg.vd_GetDeviceInfo.deviceSerial, d->serialNumber().c_str(), 127);
										resp.msg.vd_GetDeviceInfo.deviceSerial[127] = '\0';
										resp.status = ipc::ReplyStatus::Ok;
									}
								}
								_this->sendReply(message.msg.vd_GenericDeviceIdMessage.clientId, resp);
							}
							break;

						case ipc::RequestType::VirtualDevices_GetDevicePose:
							{
								ipc::Reply resp(ipc::ReplyType::VirtualDevices_GetDevicePose);
								resp.messageId = message.msg.vd_GenericDeviceIdMessage.messageId;
								if (message.msg.vd_GenericDeviceIdMessage.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									auto d = driver->virtualDevices_getDevice(message.msg.vd_GenericDeviceIdMessage.deviceId);
									if (!d) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										resp.msg.vd_GetDevicePose.pose = d->GetPose();
										resp.status = ipc::ReplyStatus::Ok;
									}
								}
								_this->sendReply(message.msg.vd_GenericDeviceIdMessage.clientId, resp);
							}
							break;

						case ipc::RequestType::VirtualDevices_GetControllerState:
							{
								ipc::Reply resp(ipc::ReplyType::VirtualDevices_GetControllerState);
								resp.messageId = message.msg.vd_GenericDeviceIdMessage.messageId;
								if (message.msg.vd_GenericDeviceIdMessage.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									auto d = driver->virtualDevices_getDevice(message.msg.vd_GenericDeviceIdMessage.deviceId);
									if (!d) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										auto c = (vr::IVRControllerComponent*)d->GetComponent(vr::IVRControllerComponent_Version);
										if (c) {
											resp.msg.vd_GetControllerState.controllerState = c->GetControllerState();
											resp.status = ipc::ReplyStatus::Ok;
										} else {
											resp.status = ipc::ReplyStatus::InvalidType;
										}
									}
								}
								_this->sendReply(message.msg.vd_GenericDeviceIdMessage.clientId, resp);
							}
							break;

						case ipc::RequestType::VirtualDevices_AddDevice:
							{
								auto result = driver->virtualDevices_addDevice(message.msg.vd_AddDevice.deviceType, message.msg.vd_AddDevice.deviceSerial);
								ipc::Reply resp(ipc::ReplyType::VirtualDevices_AddDevice);
								resp.messageId = message.msg.vd_AddDevice.messageId;
								if (result >= 0) {
									resp.status = ipc::ReplyStatus::Ok;
									resp.msg.vd_AddDevice.virtualDeviceId = (uint32_t)result;
								} else if (result == -1) {
									resp.status = ipc::ReplyStatus::TooManyDevices;
								} else if (result == -2) {
									resp.status = ipc::ReplyStatus::AlreadyInUse;
									auto d = driver->virtualDevices_findDevice(message.msg.vd_AddDevice.deviceSerial);
									resp.msg.vd_AddDevice.virtualDeviceId = d->virtualDeviceId();
								} else if (result == -3) {
									resp.status = ipc::ReplyStatus::InvalidType;
								} else {
									resp.status = ipc::ReplyStatus::UnknownError;
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while adding virtual device: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.vd_AddDevice.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::VirtualDevices_PublishDevice:
							{
								auto result = driver->virtualDevices_publishDevice(message.msg.vd_GenericDeviceIdMessage.deviceId);
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.vd_GenericDeviceIdMessage.messageId;
								if (result >= 0) {
									resp.status = ipc::ReplyStatus::Ok;
								} else if (result == -1) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else if (result == -2) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else if (result == -3) {
									resp.status = ipc::ReplyStatus::Ok; // It's already published, let's regard this as "Ok"
								} else if (result == -4) {
									resp.status = ipc::ReplyStatus::MissingProperty;
								} else {
									resp.status = ipc::ReplyStatus::UnknownError;
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while publishing virtual device: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.vd_GenericDeviceIdMessage.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::VirtualDevices_SetDeviceProperty:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.vd_SetDeviceProperty.messageId;
								if (message.msg.vd_SetDeviceProperty.virtualDeviceId >= driver->virtualDevices_getDeviceCount()) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									auto device = driver->virtualDevices_getDevice(message.msg.vd_SetDeviceProperty.virtualDeviceId);
									if (!device) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										resp.status = ipc::ReplyStatus::Ok;
										switch (message.msg.vd_SetDeviceProperty.valueType) {
										case DevicePropertyValueType::BOOL:
											LOG(TRACE) << "CTrackedDeviceDriver[" << device->serialNumber() << "]::setTrackedDeviceProperty("
												<< message.msg.vd_SetDeviceProperty.deviceProperty << ", " << message.msg.vd_SetDeviceProperty.value.boolValue << ")";
											device->setTrackedDeviceProperty(message.msg.vd_SetDeviceProperty.deviceProperty, message.msg.vd_SetDeviceProperty.value.boolValue);
											break;
										case DevicePropertyValueType::FLOAT:
											LOG(TRACE) << "CTrackedDeviceDriver[" << device->serialNumber() << "]::setTrackedDeviceProperty("
												<< message.msg.vd_SetDeviceProperty.deviceProperty << ", " << message.msg.vd_SetDeviceProperty.value.floatValue << ")";
											device->setTrackedDeviceProperty(message.msg.vd_SetDeviceProperty.deviceProperty, message.msg.vd_SetDeviceProperty.value.floatValue);
											break;
										case DevicePropertyValueType::INT32:
											LOG(TRACE) << "CTrackedDeviceDriver[" << device->serialNumber() << "]::setTrackedDeviceProperty("
												<< message.msg.vd_SetDeviceProperty.deviceProperty << ", " << message.msg.vd_SetDeviceProperty.value.int32Value << ")";
											device->setTrackedDeviceProperty(message.msg.vd_SetDeviceProperty.deviceProperty, message.msg.vd_SetDeviceProperty.value.int32Value);
											break;
										case DevicePropertyValueType::MATRIX34:
											LOG(TRACE) << "CTrackedDeviceDriver[" << device->serialNumber() << "]::setTrackedDeviceProperty("
												<< message.msg.vd_SetDeviceProperty.deviceProperty << ", <matrix34> )";
											device->setTrackedDeviceProperty(message.msg.vd_SetDeviceProperty.deviceProperty, message.msg.vd_SetDeviceProperty.value.matrix34Value);
											break;
										case DevicePropertyValueType::MATRIX44:
											LOG(TRACE) << "CTrackedDeviceDriver[" << device->serialNumber() << "]::setTrackedDeviceProperty("
												<< message.msg.vd_SetDeviceProperty.deviceProperty << ", <matrix44> )";
											device->setTrackedDeviceProperty(message.msg.vd_SetDeviceProperty.deviceProperty, message.msg.vd_SetDeviceProperty.value.matrix44Value);
											break;
										case DevicePropertyValueType::VECTOR3:
											LOG(TRACE) << "CTrackedDeviceDriver[" << device->serialNumber() << "]::setTrackedDeviceProperty("
												<< message.msg.vd_SetDeviceProperty.deviceProperty << ", <vector3> )";
											device->setTrackedDeviceProperty(message.msg.vd_SetDeviceProperty.deviceProperty, message.msg.vd_SetDeviceProperty.value.vector3Value);
											break;
										case DevicePropertyValueType::VECTOR4:
											LOG(TRACE) << "CTrackedDeviceDriver[" << device->serialNumber() << "]::setTrackedDeviceProperty("
												<< message.msg.vd_SetDeviceProperty.deviceProperty << ", <vector4> )";
											device->setTrackedDeviceProperty(message.msg.vd_SetDeviceProperty.deviceProperty, message.msg.vd_SetDeviceProperty.value.vector4Value);
											break;
										case DevicePropertyValueType::STRING:
											LOG(TRACE) << "CTrackedDeviceDriver[" << device->serialNumber() << "]::setTrackedDeviceProperty("
												<< message.msg.vd_SetDeviceProperty.deviceProperty << ", " << message.msg.vd_SetDeviceProperty.value.stringValue << ")";
											device->setTrackedDeviceProperty(message.msg.vd_SetDeviceProperty.deviceProperty, std::string(message.msg.vd_SetDeviceProperty.value.stringValue));
											break;
										case DevicePropertyValueType::UINT64:
											LOG(TRACE) << "CTrackedDeviceDriver[" << device->serialNumber() << "]::setTrackedDeviceProperty("
												<< message.msg.vd_SetDeviceProperty.deviceProperty << ", " << message.msg.vd_SetDeviceProperty.value.uint64Value << ")";
											device->setTrackedDeviceProperty(message.msg.vd_SetDeviceProperty.deviceProperty, message.msg.vd_SetDeviceProperty.value.uint64Value);
											break;
										default:
											resp.status = ipc::ReplyStatus::InvalidType;
											break;
										}
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while setting device property: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.vd_SetDeviceProperty.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::VirtualDevices_RemoveDeviceProperty:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.vd_RemoveDeviceProperty.messageId;
								if (message.msg.vd_RemoveDeviceProperty.virtualDeviceId >= driver->virtualDevices_getDeviceCount()) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									auto device = driver->virtualDevices_getDevice(message.msg.vd_RemoveDeviceProperty.virtualDeviceId);
									if (!device) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										LOG(TRACE) << "CTrackedDeviceDriver[" << device->serialNumber() << "]::removeTrackedDeviceProperty("
											<< message.msg.vd_RemoveDeviceProperty.deviceProperty << ")";
										device->removeTrackedDeviceProperty(message.msg.vd_RemoveDeviceProperty.deviceProperty);
										resp.status = ipc::ReplyStatus::Ok;
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while removing device property: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.vd_RemoveDeviceProperty.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::VirtualDevices_SetDevicePose:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.vd_SetDevicePose.messageId;
								if (message.msg.vd_SetDevicePose.virtualDeviceId >= driver->virtualDevices_getDeviceCount()) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									auto device = driver->virtualDevices_getDevice(message.msg.vd_SetDevicePose.virtualDeviceId);
									if (!device) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										auto now = std::chrono::duration_cast <std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
										auto diff = 0.0;
										if (message.timestamp < now) {
											diff = ((double)now - message.timestamp) / 1000.0;
										}
										device->updatePose(message.msg.vd_SetDevicePose.pose, -diff);
										resp.status = ipc::ReplyStatus::Ok;
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device pose: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.vd_SetDevicePose.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::VirtualDevices_SetControllerState:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.vd_SetControllerState.messageId;
								if (message.msg.vd_SetControllerState.virtualDeviceId >= driver->virtualDevices_getDeviceCount()) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									auto device = driver->virtualDevices_getDevice(message.msg.vd_SetControllerState.virtualDeviceId);
									if (!device) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										resp.status = ipc::ReplyStatus::Ok;
										if (device->deviceType() == VirtualDeviceType::TrackedController) {
											auto now = std::chrono::duration_cast <std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
											auto diff = 0.0;
											if (message.timestamp < now) {
												diff = ((double)now - message.timestamp) / 1000.0;
											}
											device->updateControllerState(message.msg.vd_SetControllerState.controllerState, -diff);
										} else {
											resp.status = ipc::ReplyStatus::InvalidType;
										}
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating controller state: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.vd_SetControllerState.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::DeviceManipulation_GetDeviceInfo:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.vd_GenericDeviceIdMessage.messageId;
								if (message.msg.vd_GenericDeviceIdMessage.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.vd_GenericDeviceIdMessage.deviceId);
									if (!info) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										resp.status = ipc::ReplyStatus::Ok;
										resp.msg.dm_deviceInfo.deviceId = message.msg.vd_GenericDeviceIdMessage.deviceId;
										resp.msg.dm_deviceInfo.deviceMode = info->deviceMode();
										resp.msg.dm_deviceInfo.deviceClass = info->deviceClass();
										auto ref = info->redirectRef();
										if (ref) {
											resp.msg.dm_deviceInfo.refDeviceId = ref->openvrId();
										} else {
											resp.msg.dm_deviceInfo.refDeviceId = (uint32_t)vr::k_unTrackedDeviceIndexInvalid;
										}
										resp.msg.dm_deviceInfo.offsetsEnabled = info->areOffsetsEnabled();
										resp.msg.dm_deviceInfo.redirectSuspended = info->redirectSuspended();
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device button mapping: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.vd_GenericDeviceIdMessage.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::InputRemapping_SetDigitalRemapping: {
							ipc::Reply resp(ipc::ReplyType::GenericReply);
							resp.messageId = message.msg.ir_SetDigitalRemapping.messageId;
							if (message.msg.ir_SetDigitalRemapping.controllerId >= vr::k_unMaxTrackedDeviceCount) {
								resp.status = ipc::ReplyStatus::InvalidId;
							} else {
								DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.ir_SetDigitalRemapping.controllerId);
								if (!info) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else {
									resp.status = ipc::ReplyStatus::Ok;
									info->setDigitalInputRemapping(message.msg.ir_SetDigitalRemapping.buttonId, message.msg.ir_SetDigitalRemapping.remapData);
								}
							}
							if (resp.status != ipc::ReplyStatus::Ok) {
								LOG(ERROR) << "Error while setting digital input remapping: Error code " << (int)resp.status;
							}
							if (resp.messageId != 0) {
								_this->sendReply(message.msg.ir_SetDigitalRemapping.clientId, resp);
							}
						} break;

						case ipc::RequestType::InputRemapping_GetDigitalRemapping: {
							ipc::Reply resp(ipc::ReplyType::InputRemapping_GetDigitalRemapping);
							resp.messageId = message.msg.ir_GetDigitalRemapping.messageId;
							if (message.msg.ir_GetDigitalRemapping.controllerId >= vr::k_unMaxTrackedDeviceCount) {
								resp.status = ipc::ReplyStatus::InvalidId;
							} else {
								DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.ir_GetDigitalRemapping.controllerId);
								if (!info) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else {
									resp.status = ipc::ReplyStatus::Ok;
									resp.msg.ir_getDigitalRemapping.deviceId = message.msg.ir_GetDigitalRemapping.controllerId;
									resp.msg.ir_getDigitalRemapping.buttonId = message.msg.ir_GetDigitalRemapping.buttonId;
									resp.msg.ir_getDigitalRemapping.remapData = info->getDigitalInputRemapping(message.msg.ir_GetDigitalRemapping.buttonId);
								}
							}
							if (resp.status != ipc::ReplyStatus::Ok) {
								LOG(ERROR) << "Error while getting digital input remapping: Error code " << (int)resp.status;
							}
							if (resp.messageId != 0) {
								_this->sendReply(message.msg.ir_GetDigitalRemapping.clientId, resp);
							}
						} break;

						case ipc::RequestType::InputRemapping_SetAnalogRemapping: {
							ipc::Reply resp(ipc::ReplyType::GenericReply);
							resp.messageId = message.msg.ir_SetAnalogRemapping.messageId;
							if (message.msg.ir_SetAnalogRemapping.controllerId >= vr::k_unMaxTrackedDeviceCount) {
								resp.status = ipc::ReplyStatus::InvalidId;
							} else {
								DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.ir_SetAnalogRemapping.controllerId);
								if (!info) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else {
									resp.status = ipc::ReplyStatus::Ok;
									info->setAnalogInputRemapping(message.msg.ir_SetAnalogRemapping.axisId, message.msg.ir_SetAnalogRemapping.remapData);
								}
							}
							if (resp.status != ipc::ReplyStatus::Ok) {
								LOG(ERROR) << "Error while setting analog input remapping: Error code " << (int)resp.status;
							}
							if (resp.messageId != 0) {
								_this->sendReply(message.msg.ir_SetAnalogRemapping.clientId, resp);
							}
						} break;

						case ipc::RequestType::InputRemapping_GetAnalogRemapping: {
							ipc::Reply resp(ipc::ReplyType::InputRemapping_GetAnalogRemapping);
							resp.messageId = message.msg.ir_GetAnalogRemapping.messageId;
							if (message.msg.ir_GetAnalogRemapping.controllerId >= vr::k_unMaxTrackedDeviceCount) {
								resp.status = ipc::ReplyStatus::InvalidId;
							} else {
								DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.ir_GetAnalogRemapping.controllerId);
								if (!info) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else {
									resp.status = ipc::ReplyStatus::Ok;
									resp.msg.ir_getAnalogRemapping.deviceId = message.msg.ir_GetAnalogRemapping.controllerId;
									resp.msg.ir_getAnalogRemapping.axisId = message.msg.ir_GetAnalogRemapping.axisId;
									resp.msg.ir_getAnalogRemapping.remapData = info->getAnalogInputRemapping(message.msg.ir_GetAnalogRemapping.axisId);
								}
							}
							if (resp.status != ipc::ReplyStatus::Ok) {
								LOG(ERROR) << "Error while getting analog input remapping: Error code " << (int)resp.status;
							}
							if (resp.messageId != 0) {
								_this->sendReply(message.msg.ir_GetAnalogRemapping.clientId, resp);
							}
						} break;

						case ipc::RequestType::DeviceManipulation_GetDeviceOffsets:
							{
								ipc::Reply resp(ipc::ReplyType::DeviceManipulation_GetDeviceOffsets);
								resp.messageId = message.msg.vd_GenericDeviceIdMessage.messageId;
								if (message.msg.vd_GenericDeviceIdMessage.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.vd_GenericDeviceIdMessage.deviceId);
									if (!info) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										resp.status = ipc::ReplyStatus::Ok;
										resp.msg.dm_deviceOffsets.deviceId = message.msg.vd_GenericDeviceIdMessage.deviceId;
										resp.msg.dm_deviceOffsets.offsetsEnabled = info->areOffsetsEnabled();
										resp.msg.dm_deviceOffsets.worldFromDriverRotationOffset = info->worldFromDriverRotationOffset();
										resp.msg.dm_deviceOffsets.worldFromDriverTranslationOffset = info->worldFromDriverTranslationOffset();
										resp.msg.dm_deviceOffsets.driverFromHeadRotationOffset = info->driverFromHeadRotationOffset();
										resp.msg.dm_deviceOffsets.driverFromHeadTranslationOffset = info->driverFromHeadTranslationOffset();
										resp.msg.dm_deviceOffsets.driverFromHeadTranslationOffset = info->driverFromHeadTranslationOffset();
										resp.msg.dm_deviceOffsets.deviceRotationOffset = info->deviceRotationOffset();
										resp.msg.dm_deviceOffsets.deviceTranslationOffset = info->deviceTranslationOffset();
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device button mapping: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.vd_GenericDeviceIdMessage.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::DeviceManipulation_SetDeviceOffsets:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.dm_DeviceOffsets.messageId;
								if (message.msg.dm_DeviceOffsets.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.dm_DeviceOffsets.deviceId);
									if (!info) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										resp.status = ipc::ReplyStatus::Ok;
										if (message.msg.dm_DeviceOffsets.enableOffsets > 0) {
											info->enableOffsets(message.msg.dm_DeviceOffsets.enableOffsets == 1 ? true : false);
										}
										switch (message.msg.dm_DeviceOffsets.offsetOperation) {
										case 0:
											if (message.msg.dm_DeviceOffsets.worldFromDriverRotationOffsetValid) {
												info->worldFromDriverRotationOffset() = message.msg.dm_DeviceOffsets.worldFromDriverRotationOffset;
											}
											if (message.msg.dm_DeviceOffsets.worldFromDriverTranslationOffsetValid) {
												info->worldFromDriverTranslationOffset() = message.msg.dm_DeviceOffsets.worldFromDriverTranslationOffset;
											}
											if (message.msg.dm_DeviceOffsets.driverFromHeadRotationOffsetValid) {
												info->driverFromHeadRotationOffset() = message.msg.dm_DeviceOffsets.driverFromHeadRotationOffset;
											}
											if (message.msg.dm_DeviceOffsets.driverFromHeadTranslationOffsetValid) {
												info->driverFromHeadTranslationOffset() = message.msg.dm_DeviceOffsets.driverFromHeadTranslationOffset;
											}
											if (message.msg.dm_DeviceOffsets.deviceRotationOffsetValid) {
												info->deviceRotationOffset() = message.msg.dm_DeviceOffsets.deviceRotationOffset;
											}
											if (message.msg.dm_DeviceOffsets.deviceTranslationOffsetValid) {
												info->deviceTranslationOffset() = message.msg.dm_DeviceOffsets.deviceTranslationOffset;
											}
											break;
										case 1:
											if (message.msg.dm_DeviceOffsets.worldFromDriverRotationOffsetValid) {
												info->worldFromDriverRotationOffset() =  message.msg.dm_DeviceOffsets.worldFromDriverRotationOffset * info->worldFromDriverRotationOffset();
											}
											if (message.msg.dm_DeviceOffsets.worldFromDriverTranslationOffsetValid) {
												info->worldFromDriverTranslationOffset() = info->worldFromDriverTranslationOffset() + message.msg.dm_DeviceOffsets.worldFromDriverTranslationOffset;
											}
											if (message.msg.dm_DeviceOffsets.driverFromHeadRotationOffsetValid) {
												info->driverFromHeadRotationOffset() = message.msg.dm_DeviceOffsets.driverFromHeadRotationOffset * info->driverFromHeadRotationOffset();
											}
											if (message.msg.dm_DeviceOffsets.driverFromHeadTranslationOffsetValid) {
												info->driverFromHeadTranslationOffset() = info->driverFromHeadTranslationOffset() + message.msg.dm_DeviceOffsets.driverFromHeadTranslationOffset;
											}
											if (message.msg.dm_DeviceOffsets.deviceRotationOffsetValid) {
												info->deviceRotationOffset() = message.msg.dm_DeviceOffsets.deviceRotationOffset * info->deviceRotationOffset();
											}
											if (message.msg.dm_DeviceOffsets.deviceTranslationOffsetValid) {
												info->deviceTranslationOffset() = info->deviceTranslationOffset() + message.msg.dm_DeviceOffsets.deviceTranslationOffset;
											}
											break;
										}
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device pose offset: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.dm_DeviceOffsets.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::DeviceManipulation_DefaultMode:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.vd_GenericDeviceIdMessage.messageId;
								if (message.msg.vd_GenericDeviceIdMessage.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.vd_GenericDeviceIdMessage.deviceId);
									if (!info) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										info->setDefaultMode();
										resp.status = ipc::ReplyStatus::Ok;
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device pose offset: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.vd_GenericDeviceIdMessage.clientId, resp);
								}
							}
							break;
								
						case ipc::RequestType::DeviceManipulation_RedirectMode:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.dm_RedirectMode.messageId;
								if (message.msg.dm_RedirectMode.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.dm_RedirectMode.deviceId);
									if (!info) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										DeviceManipulationHandle* infoTarget = driver->getDeviceManipulationHandleById(message.msg.dm_RedirectMode.targetId);
										if (info && infoTarget) {
											if (info->deviceMode() > 0) {
												info->setDefaultMode();
											}
											if (infoTarget->deviceMode() > 0) {
												infoTarget->setDefaultMode();
											}
											info->setRedirectMode(false, infoTarget);
											infoTarget->setRedirectMode(true, info);
											resp.status = ipc::ReplyStatus::Ok;
										} else {
											resp.status = ipc::ReplyStatus::UnknownError;
										}
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device pose offset: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.dm_RedirectMode.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::DeviceManipulation_SwapMode:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.dm_SwapMode.messageId;
								if (message.msg.dm_SwapMode.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.dm_SwapMode.deviceId);
									if (!info) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										DeviceManipulationHandle* infoTarget = driver->getDeviceManipulationHandleById(message.msg.dm_SwapMode.targetId);
										if (info && infoTarget) {
											if (info->deviceMode() > 0) {
												info->setDefaultMode();
											}
											if (infoTarget->deviceMode() > 0) {
												infoTarget->setDefaultMode();
											}
											info->setSwapMode(infoTarget);
											infoTarget->setSwapMode(info);
											resp.status = ipc::ReplyStatus::Ok;
										} else {
											resp.status = ipc::ReplyStatus::UnknownError;
										}
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device pose offset: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.dm_SwapMode.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::DeviceManipulation_MotionCompensationMode:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.dm_MotionCompensationMode.messageId;
								if (message.msg.dm_MotionCompensationMode.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.dm_MotionCompensationMode.deviceId);
									if (!info) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										auto serverDriver = ServerDriver::getInstance();
										if (serverDriver) {
											serverDriver->motionCompensation().setMotionCompensationVelAccMode(message.msg.dm_MotionCompensationMode.velAccCompensationMode);
											info->setMotionCompensationMode();
											_this->_setMotionCompensationMessageId = message.msg.dm_MotionCompensationMode.messageId;
											_this->_setMotionCompensationClientId = message.msg.dm_MotionCompensationMode.clientId;
											resp.status = ipc::ReplyStatus::Ok;
										} else {
											resp.status = ipc::ReplyStatus::UnknownError;
										}
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device pose offset: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0 && resp.status != ipc::ReplyStatus::Ok) {
									_this->sendReply(message.msg.dm_MotionCompensationMode.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::DeviceManipulation_FakeDisconnectedMode:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.vd_GenericDeviceIdMessage.messageId;
								if (message.msg.vd_GenericDeviceIdMessage.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.vd_GenericDeviceIdMessage.deviceId);
									if (!info) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										info->setFakeDisconnectedMode();
										resp.status = ipc::ReplyStatus::Ok;
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device pose offset: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.vd_GenericDeviceIdMessage.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::DeviceManipulation_TriggerHapticPulse:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.dm_triggerHapticPulse.messageId;
								if (message.msg.dm_triggerHapticPulse.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else {
									DeviceManipulationHandle* info = driver->getDeviceManipulationHandleById(message.msg.dm_triggerHapticPulse.deviceId);
									if (!info) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										info->triggerHapticPulse(message.msg.dm_triggerHapticPulse.axisId, message.msg.dm_triggerHapticPulse.durationMicroseconds, message.msg.dm_triggerHapticPulse.directMode);
										resp.status = ipc::ReplyStatus::Ok;
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while triggering haptic pulse: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.dm_triggerHapticPulse.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::DeviceManipulation_SetMotionCompensationProperties:
							{
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.dm_SetMotionCompensationProperties.messageId;
								auto serverDriver = ServerDriver::getInstance();
								if (serverDriver) {
									if (message.msg.dm_SetMotionCompensationProperties.velAccCompensationModeValid) {
										serverDriver->motionCompensation().setMotionCompensationVelAccMode(message.msg.dm_SetMotionCompensationProperties.velAccCompensationMode);
									}
									if (message.msg.dm_SetMotionCompensationProperties.kalmanFilterProcessNoiseValid) {
										serverDriver->motionCompensation().setMotionCompensationKalmanProcessVariance(message.msg.dm_SetMotionCompensationProperties.kalmanFilterProcessNoise);
									}
									if (message.msg.dm_SetMotionCompensationProperties.kalmanFilterObservationNoiseValid) {
										serverDriver->motionCompensation().setMotionCompensationKalmanObservationVariance(message.msg.dm_SetMotionCompensationProperties.kalmanFilterObservationNoise);
									}
									if (message.msg.dm_SetMotionCompensationProperties.movingAverageWindowValid) {
										serverDriver->motionCompensation().setMotionCompensationMovingAverageWindow(message.msg.dm_SetMotionCompensationProperties.movingAverageWindow);
									}
									resp.status = ipc::ReplyStatus::Ok;
								} else {
									resp.status = ipc::ReplyStatus::UnknownError;
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while setting motion compensation properties: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									_this->sendReply(message.msg.dm_SetMotionCompensationProperties.clientId, resp);
								}
							}
							break;

						case ipc::RequestType::InputRemapping_SetTouchpadEmulationFixEnabled: {
							DeviceManipulationHandle::setTouchpadEmulationFixFlag(message.msg.ir_SetTouchPadEmulationFixEnabled.enable);
						} break;

						default:
							LOG(ERROR) << "Error in ipc server receive loop: Unknown message type (" << (int)message.type << ")";
							break;
						}
					} else {
						LOG(ERROR) << "Error in ipc server receive loop: received size is wrong (" << recv_size << " != " << sizeof(ipc::Request) << ")";
					}
				}
			} catch (std::exception& ex) {
				LOG(ERROR) << "Exception caught in ipc server receive loop: " << ex.what();
			}
		}
		boost::interprocess::message_queue::remove(_this->_ipcQueueName.c_str());
	} catch (std::exception& ex) {
		LOG(ERROR) << "Exception caught in ipc server thread: " << ex.what();
	}
	_this->_ipcThreadRunning = false;
	LOG(DEBUG) << "CServerDriver::_ipcThreadFunc: thread stopped";
}


void IpcShmCommunicator::sendReply(uint32_t clientId, const ipc::Reply& reply) {
	std::lock_guard<std::mutex> guard(_sendMutex);
	auto i = _ipcEndpoints.find(clientId);
	if (i != _ipcEndpoints.end()) {
		i->second->send(&reply, sizeof(ipc::Reply), 0);
	} else {
		LOG(ERROR) << "Error while sending reply: Unknown clientId " << clientId;
	}
}


} // end namespace driver
} // end namespace vrinputemulator
