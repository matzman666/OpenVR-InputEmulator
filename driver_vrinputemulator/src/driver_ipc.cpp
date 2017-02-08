
#include "stdafx.h"
#include "driver_vrinputemulator.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <ipc_protocol.h>

namespace vrinputemulator {
namespace driver {

void CServerDriver::_ipcThreadFunc(CServerDriver * _this) {
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
									if (message.msg.ipc_ClientConnect.ipcProcotolVersion == IPC_PROTOCOL_VERSION) {
										auto clientId = _this->_ipcClientIdNext++;
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
									queue->send(&reply, sizeof(ipc::Reply), 0);
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
									auto msgQueue = i->second;
									_this->_ipcEndpoints.erase(i);
									LOG(INFO) << "Client disconnected: clientId " << message.msg.ipc_ClientDisconnect.clientId;
									if (reply.messageId != 0) {
										msgQueue->send(&reply, sizeof(ipc::Reply), 0);
									}
								} else {
									LOG(ERROR) << "Error during client disconnect: unknown clientID " << message.msg.ipc_ClientDisconnect.clientId;
								}
							}
							break;

						case ipc::RequestType::IPC_Ping:
							{
								LOG(TRACE) << "Ping received: clientId " << message.msg.ipc_Ping.clientId << ", nonce " << message.msg.ipc_Ping.nonce;
								auto i = _this->_ipcEndpoints.find(message.msg.ipc_Ping.clientId);
								if (i != _this->_ipcEndpoints.end()) {
									ipc::Reply reply(ipc::ReplyType::IPC_Ping);
									reply.messageId = message.msg.ipc_Ping.messageId;
									reply.status = ipc::ReplyStatus::Ok;
									reply.msg.ipc_Ping.nonce = message.msg.ipc_Ping.nonce;
									if (reply.messageId != 0) {
										i->second->send(&reply, sizeof(ipc::Reply), 0);
									}
								} else {
									LOG(ERROR) << "Error during ping: unknown clientID " << message.msg.ipc_ClientDisconnect.clientId;
								}
							}
							break;

						case ipc::RequestType::OpenVR_ButtonEvent:
							{
								if (vr::VRServerDriverHost()) {
									unsigned iterCount = min(message.msg.ipc_ButtonEvent.eventCount, REQUEST_OPENVR_BUTTONEVENT_MAXCOUNT);
									for (unsigned i = 0; i < iterCount; ++i) {
										auto& e = message.msg.ipc_ButtonEvent.events[i];
										auto devicePtr = _this->m_deviceIdMap[e.deviceId];
										if (devicePtr && devicePtr->deviceType() == VirtualDeviceType::TrackedController) {
											((CTrackedControllerDriver*)devicePtr)->buttonEvent(e.eventType, e.buttonId, e.timeOffset);
										} else {
											vr::IVRServerDriverHost* driverHost;
											if (_trackedDeviceInfos[e.deviceId] && _trackedDeviceInfos[e.deviceId]->isValid) {
												driverHost = _trackedDeviceInfos[e.deviceId]->driverHost;
											} else {
												driverHost = vr::VRServerDriverHost();
											}
											switch (e.eventType) {
											case ButtonEventType::ButtonPressed:
												driverHost->TrackedDeviceButtonPressed(e.deviceId, e.buttonId, e.timeOffset);
												break;
											case ButtonEventType::ButtonUnpressed:
												driverHost->TrackedDeviceButtonUnpressed(e.deviceId, e.buttonId, e.timeOffset);
												break;
											case ButtonEventType::ButtonTouched:
												driverHost->TrackedDeviceButtonTouched(e.deviceId, e.buttonId, e.timeOffset);
												break;
											case ButtonEventType::ButtonUntouched:
												driverHost->TrackedDeviceButtonUntouched(e.deviceId, e.buttonId, e.timeOffset);
												break;
											default:
												LOG(ERROR) << "Error in ipc server receive loop: Unknown button event type (" << (int)e.eventType << ")";
												break;
											}
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
										auto devicePtr = _this->m_deviceIdMap[e.deviceId];
										if (devicePtr && devicePtr->deviceType() == VirtualDeviceType::TrackedController) {
											((CTrackedControllerDriver*)devicePtr)->axisEvent(e.axisId, e.axisState);
										} else {
											vr::IVRServerDriverHost* driverHost;
											if (_trackedDeviceInfos[e.deviceId] && _trackedDeviceInfos[e.deviceId]->isValid) {
												driverHost = _trackedDeviceInfos[e.deviceId]->driverHost;
											} else {
												driverHost = vr::VRServerDriverHost();
											}
											driverHost->TrackedDeviceAxisUpdated(e.deviceId, e.axisId, e.axisState);
										}
									}
								}
							}
							break;

						case ipc::RequestType::OpenVR_PoseUpdate:
							{
								if (vr::VRServerDriverHost()) {
									auto devicePtr = _this->m_deviceIdMap[message.msg.ipc_PoseUpdate.deviceId];
									auto now = std::chrono::duration_cast <std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
									auto diff = 0.0;
									if (message.timestamp < now) {
										diff = ((double)now - message.timestamp) / 1000.0;
									}
									if (devicePtr) {
										devicePtr->updatePose(message.msg.ipc_PoseUpdate.pose, -diff);
									} else {
										vr::IVRServerDriverHost* driverHost;
										if (_trackedDeviceInfos[message.msg.ipc_PoseUpdate.deviceId] && _trackedDeviceInfos[message.msg.ipc_PoseUpdate.deviceId]->isValid) {
											driverHost = _trackedDeviceInfos[message.msg.ipc_PoseUpdate.deviceId]->driverHost;
										} else {
											driverHost = vr::VRServerDriverHost();
										}
										message.msg.ipc_PoseUpdate.pose.poseTimeOffset -= diff;
										driverHost->TrackedDevicePoseUpdated(message.msg.ipc_PoseUpdate.deviceId, message.msg.ipc_PoseUpdate.pose, sizeof(vr::DriverPose_t));
									}
								}
							}
							break;

						case ipc::RequestType::OpenVR_ProximitySensorEvent:
							{
								vr::IVRServerDriverHost* driverHost;
								if (_trackedDeviceInfos[message.msg.ipc_PoseUpdate.deviceId] && _trackedDeviceInfos[message.msg.ovr_ProximitySensorEvent.deviceId]->isValid) {
									driverHost = _trackedDeviceInfos[message.msg.ovr_ProximitySensorEvent.deviceId]->driverHost;
								} else {
									driverHost = vr::VRServerDriverHost();
								}
								driverHost->ProximitySensorState(message.msg.ovr_ProximitySensorEvent.deviceId, message.msg.ovr_ProximitySensorEvent.sensorTriggered);
							}
							break;

						case ipc::RequestType::OpenVR_VendorSpecificEvent:
							{
								vr::IVRServerDriverHost* driverHost;
								if (_trackedDeviceInfos[message.msg.ipc_PoseUpdate.deviceId] && _trackedDeviceInfos[message.msg.ovr_VendorSpecificEvent.deviceId]->isValid) {
									driverHost = _trackedDeviceInfos[message.msg.ovr_VendorSpecificEvent.deviceId]->driverHost;
								} else {
									driverHost = vr::VRServerDriverHost();
								}
								driverHost->VendorSpecificEvent(message.msg.ovr_VendorSpecificEvent.deviceId, message.msg.ovr_VendorSpecificEvent.eventType,
										message.msg.ovr_VendorSpecificEvent.eventData, message.msg.ovr_VendorSpecificEvent.timeOffset);
							}
							break;

						case ipc::RequestType::VirtualDevices_GetDeviceCount:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								auto i = _this->_ipcEndpoints.find(message.msg.vd_GenericClientMessage.clientId);
								if (i != _this->_ipcEndpoints.end()) {
									ipc::Reply resp(ipc::ReplyType::VirtualDevices_GetDeviceCount);
									resp.messageId = message.msg.vd_GenericClientMessage.messageId;
									resp.status = ipc::ReplyStatus::Ok;
									resp.msg.vd_GetDeviceCount.deviceCount = _this->m_emulatedDeviceCount;
									i->second->send(&resp, sizeof(ipc::Reply), 0);
								} else {
									LOG(ERROR) << "Error while adding tracked device: Unknown clientId " << message.msg.vd_AddDevice.clientId;
								}

							}
							break;

						case ipc::RequestType::VirtualDevices_GetDeviceInfo:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								auto i = _this->_ipcEndpoints.find(message.msg.vd_GenericDeviceIdMessage.clientId);
								if (i != _this->_ipcEndpoints.end()) {
									ipc::Reply resp(ipc::ReplyType::VirtualDevices_GetDeviceInfo);
									resp.messageId = message.msg.vd_GenericDeviceIdMessage.messageId;
									if (message.msg.vd_GenericDeviceIdMessage.deviceId >= vr::k_unMaxTrackedDeviceCount) {
										resp.status = ipc::ReplyStatus::InvalidId;
									} else if (!_this->m_emulatedDevices[message.msg.vd_GenericDeviceIdMessage.deviceId]) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										auto d = _this->m_emulatedDevices[message.msg.vd_GenericDeviceIdMessage.deviceId].get();
										resp.msg.vd_GetDeviceInfo.virtualDeviceId = message.msg.vd_GenericDeviceIdMessage.deviceId;
										resp.msg.vd_GetDeviceInfo.openvrDeviceId = d->openvrDeviceId();
										resp.msg.vd_GetDeviceInfo.deviceType = d->deviceType();
										strncpy_s(resp.msg.vd_GetDeviceInfo.deviceSerial, d->serialNumber().c_str(), 127);
										resp.msg.vd_GetDeviceInfo.deviceSerial[127] = '\0';
										resp.status = ipc::ReplyStatus::Ok;
									}
									i->second->send(&resp, sizeof(ipc::Reply), 0);
								} else {
									LOG(ERROR) << "Error while adding tracked device: Unknown clientId " << message.msg.vd_AddDevice.clientId;
								}

							}
							break;

						case ipc::RequestType::VirtualDevices_GetDevicePose:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								auto i = _this->_ipcEndpoints.find(message.msg.vd_GenericDeviceIdMessage.clientId);
								if (i != _this->_ipcEndpoints.end()) {
									ipc::Reply resp(ipc::ReplyType::VirtualDevices_GetDevicePose);
									resp.messageId = message.msg.vd_GenericDeviceIdMessage.messageId;
									if (message.msg.vd_GenericDeviceIdMessage.deviceId >= vr::k_unMaxTrackedDeviceCount) {
										resp.status = ipc::ReplyStatus::InvalidId;
									} else if (!_this->m_emulatedDevices[message.msg.vd_GenericDeviceIdMessage.deviceId]) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										auto d = _this->m_emulatedDevices[message.msg.vd_GenericDeviceIdMessage.deviceId].get();
										resp.msg.vd_GetDevicePose.pose = d->GetPose();
										resp.status = ipc::ReplyStatus::Ok;
									}
									i->second->send(&resp, sizeof(ipc::Reply), 0);
								} else {
									LOG(ERROR) << "Error while adding tracked device: Unknown clientId " << message.msg.vd_AddDevice.clientId;
								}

							}
							break;

						case ipc::RequestType::VirtualDevices_GetControllerState:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								auto i = _this->_ipcEndpoints.find(message.msg.vd_GenericDeviceIdMessage.clientId);
								if (i != _this->_ipcEndpoints.end()) {
									ipc::Reply resp(ipc::ReplyType::VirtualDevices_GetControllerState);
									resp.messageId = message.msg.vd_GenericDeviceIdMessage.messageId;
									if (message.msg.vd_GenericDeviceIdMessage.deviceId >= vr::k_unMaxTrackedDeviceCount) {
										resp.status = ipc::ReplyStatus::InvalidId;
									} else if (!_this->m_emulatedDevices[message.msg.vd_GenericDeviceIdMessage.deviceId]) {
										resp.status = ipc::ReplyStatus::NotFound;
									} else {
										auto d = (vr::IVRControllerComponent*)_this->m_emulatedDevices[message.msg.vd_GenericDeviceIdMessage.deviceId].get()->GetComponent(vr::IVRControllerComponent_Version);
										if (d) {
											resp.msg.vd_GetControllerState.controllerState = d->GetControllerState();
											resp.status = ipc::ReplyStatus::Ok;
										} else {
											resp.status = ipc::ReplyStatus::InvalidType;
										}
									}
									i->second->send(&resp, sizeof(ipc::Reply), 0);
								} else {
									LOG(ERROR) << "Error while adding tracked device: Unknown clientId " << message.msg.vd_AddDevice.clientId;
								}

							}
							break;

						case ipc::RequestType::VirtualDevices_AddDevice:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								auto i = _this->_ipcEndpoints.find(message.msg.vd_AddDevice.clientId);
								if (i != _this->_ipcEndpoints.end()) {
									auto result = _this->addTrackedDevice(message.msg.vd_AddDevice.deviceType, message.msg.vd_AddDevice.deviceSerial);
									ipc::Reply resp(ipc::ReplyType::VirtualDevices_AddDevice);
									resp.messageId = message.msg.vd_AddDevice.messageId;
									if (result >= 0) {
										resp.status = ipc::ReplyStatus::Ok;
										resp.msg.vd_AddDevice.virtualDeviceId = (uint32_t)result;
									} else if (result == -1) {
										resp.status = ipc::ReplyStatus::TooManyDevices;
									} else if (result == -2) {
										resp.status = ipc::ReplyStatus::AlreadyInUse;
										for (uint32_t i = 0; i < _this->m_emulatedDeviceCount; ++i) {
											if (_this->m_emulatedDevices[i]->serialNumber().compare(message.msg.vd_AddDevice.deviceSerial) == 0) {
												resp.msg.vd_AddDevice.virtualDeviceId = i;
												break;
											}
										}
									} else if (result == -3) {
										resp.status = ipc::ReplyStatus::InvalidType;
									} else {
										resp.status = ipc::ReplyStatus::UnknownError;
									}
									if (resp.status != ipc::ReplyStatus::Ok) {
										LOG(ERROR) << "Error while adding tracked device: Error code " << (int)resp.status;
									}
									if (resp.messageId != 0) {
										i->second->send(&resp, sizeof(ipc::Reply), 0);
									}
								} else {
									LOG(ERROR) << "Error while adding tracked device: Unknown clientId " << message.msg.vd_AddDevice.clientId;
								}
							}
							break;

						case ipc::RequestType::VirtualDevices_PublishDevice:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								auto result = _this->publishTrackedDevice(message.msg.vd_GenericDeviceIdMessage.deviceId);
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
									LOG(ERROR) << "Error while publishing tracked device: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									auto i = _this->_ipcEndpoints.find(message.msg.vd_GenericDeviceIdMessage.clientId);
									if (i != _this->_ipcEndpoints.end()) {
										i->second->send(&resp, sizeof(ipc::Reply), 0);
									} else {
										LOG(ERROR) << "Error while publishing tracked device: Unknown clientId " << message.msg.vd_GenericDeviceIdMessage.clientId;
									}
								}
							}
							break;

						case ipc::RequestType::VirtualDevices_SetDeviceProperty:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.vd_SetDeviceProperty.messageId;
								if (message.msg.vd_SetDeviceProperty.virtualDeviceId >= _this->m_emulatedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else if (!_this->m_emulatedDevices[message.msg.vd_SetDeviceProperty.virtualDeviceId]) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else {
									auto device = _this->m_emulatedDevices[message.msg.vd_SetDeviceProperty.virtualDeviceId].get();
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
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while setting device property: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									auto i = _this->_ipcEndpoints.find(message.msg.vd_SetDeviceProperty.clientId);
									if (i != _this->_ipcEndpoints.end()) {
										i->second->send(&resp, sizeof(ipc::Reply), 0);
									} else {
										LOG(ERROR) << "Error while setting device property: Unknown clientId " << message.msg.vd_SetDeviceProperty.clientId;
									}
								}
							}
							break;

						case ipc::RequestType::VirtualDevices_RemoveDeviceProperty:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.vd_RemoveDeviceProperty.messageId;
								if (message.msg.vd_RemoveDeviceProperty.virtualDeviceId >= _this->m_emulatedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else if (!_this->m_emulatedDevices[message.msg.vd_RemoveDeviceProperty.virtualDeviceId]) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else {
									auto device = _this->m_emulatedDevices[message.msg.vd_RemoveDeviceProperty.virtualDeviceId].get();
									LOG(TRACE) << "CTrackedDeviceDriver[" << device->serialNumber() << "]::removeTrackedDeviceProperty("
										<< message.msg.vd_RemoveDeviceProperty.deviceProperty << ")";
									device->removeTrackedDeviceProperty(message.msg.vd_RemoveDeviceProperty.deviceProperty);
									resp.status = ipc::ReplyStatus::Ok;
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while removing device property: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									auto i = _this->_ipcEndpoints.find(message.msg.vd_RemoveDeviceProperty.clientId);
									if (i != _this->_ipcEndpoints.end()) {
										i->second->send(&resp, sizeof(ipc::Reply), 0);
									} else {
										LOG(ERROR) << "Error while removing device property: Unknown clientId " << message.msg.vd_RemoveDeviceProperty.clientId;
									}
								}
							}
							break;

						case ipc::RequestType::VirtualDevices_SetDevicePose:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.vd_SetDevicePose.messageId;
								if (message.msg.vd_SetDevicePose.virtualDeviceId >= _this->m_emulatedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else if (!_this->m_emulatedDevices[message.msg.vd_SetDevicePose.virtualDeviceId]) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else {
									auto device = _this->m_emulatedDevices[message.msg.vd_SetDevicePose.virtualDeviceId].get();
									auto now = std::chrono::duration_cast <std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
									auto diff = 0.0;
									if (message.timestamp < now) {
										diff = ((double)now - message.timestamp) / 1000.0;
									}
									device->updatePose(message.msg.vd_SetDevicePose.pose, -diff);
									resp.status = ipc::ReplyStatus::Ok;
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device pose: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									auto i = _this->_ipcEndpoints.find(message.msg.vd_SetDevicePose.clientId);
									if (i != _this->_ipcEndpoints.end()) {
										i->second->send(&resp, sizeof(ipc::Reply), 0);
									} else {
										LOG(ERROR) << "Error while updating device pose: Unknown clientId " << message.msg.vd_SetDevicePose.clientId;
									}
								}
							}
							break;

						case ipc::RequestType::VirtualDevices_SetControllerState:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.vd_SetControllerState.messageId;
								if (message.msg.vd_SetControllerState.virtualDeviceId >= _this->m_emulatedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else if (!_this->m_emulatedDevices[message.msg.vd_SetControllerState.virtualDeviceId]) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else {
									auto device = _this->m_emulatedDevices[message.msg.vd_SetControllerState.virtualDeviceId].get();
									resp.status = ipc::ReplyStatus::Ok;
									if (device->deviceType() == VirtualDeviceType::TrackedController) {
										auto controller = (CTrackedControllerDriver*)device;
										auto now = std::chrono::duration_cast <std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
										auto diff = 0.0;
										if (message.timestamp < now) {
											diff = ((double)now - message.timestamp) / 1000.0;
										}
										controller->updateControllerState(message.msg.vd_SetControllerState.controllerState, -diff);
									} else {
										resp.status = ipc::ReplyStatus::InvalidType;
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating controller state: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									auto i = _this->_ipcEndpoints.find(message.msg.vd_SetControllerState.clientId);
									if (i != _this->_ipcEndpoints.end()) {
										i->second->send(&resp, sizeof(ipc::Reply), 0);
									} else {
										LOG(ERROR) << "Error while updating controller state: Unknown clientId " << message.msg.vd_SetControllerState.clientId;
									}
								}
							}
							break;

						case ipc::RequestType::DeviceManipulation_ButtonMapping:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.dm_ButtonMapping.messageId;
								if (message.msg.dm_ButtonMapping.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else if (!_trackedDeviceInfos[message.msg.dm_ButtonMapping.deviceId] || !_trackedDeviceInfos[message.msg.dm_ButtonMapping.deviceId]->isValid) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else {
									resp.status = ipc::ReplyStatus::Ok;
									auto device = _trackedDeviceInfos[message.msg.dm_ButtonMapping.deviceId];
									if (message.msg.dm_ButtonMapping.enableMapping > 0) {
										device->enableButtonMapping = message.msg.dm_ButtonMapping.enableMapping == 1 ? true : false;
									}
									switch (message.msg.dm_ButtonMapping.mappingOperation) {
										case 0:
											break;
										case 1:
											for (unsigned i = 0; i < message.msg.dm_ButtonMapping.mappingCount; ++i) {
												device->buttonMapping[message.msg.dm_ButtonMapping.buttonMappings[i * 2]] = message.msg.dm_ButtonMapping.buttonMappings[i * 2 + 1];
											}
											break;
										case 2:
											for (unsigned i = 0; i < message.msg.dm_ButtonMapping.mappingCount; ++i) {
												device->buttonMapping.erase(message.msg.dm_ButtonMapping.buttonMappings[i]);
											}
											break;
										case 3:
											device->buttonMapping.clear();
											break;
										default:
											resp.status = ipc::ReplyStatus::InvalidOperation;
											break;
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device button mapping: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									auto i = _this->_ipcEndpoints.find(message.msg.dm_ButtonMapping.clientId);
									if (i != _this->_ipcEndpoints.end()) {
										i->second->send(&resp, sizeof(ipc::Reply), 0);
									} else {
										LOG(ERROR) << "Error while updating device button mapping: Unknown clientId " << message.msg.dm_ButtonMapping.clientId;
									}
								}
							}
							break;

						case ipc::RequestType::DeviceManipulation_PoseOffset:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.dm_PoseOffset.messageId;
								if (message.msg.dm_PoseOffset.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else if (!_trackedDeviceInfos[message.msg.dm_PoseOffset.deviceId] || !_trackedDeviceInfos[message.msg.dm_PoseOffset.deviceId]->isValid) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else {
									resp.status = ipc::ReplyStatus::Ok;
									auto device = _trackedDeviceInfos[message.msg.dm_PoseOffset.deviceId];
									if (message.msg.dm_PoseOffset.enableOffset > 0) {
										device->enablePoseOffset = message.msg.dm_PoseOffset.enableOffset == 1 ? true : false;
									}
									if (message.msg.dm_PoseOffset.offsetValid) {
										device->poseOffset[0] = message.msg.dm_PoseOffset.offset[0];
										device->poseOffset[1] = message.msg.dm_PoseOffset.offset[1];
										device->poseOffset[2] = message.msg.dm_PoseOffset.offset[2];
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device pose offset: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									auto i = _this->_ipcEndpoints.find(message.msg.dm_PoseOffset.clientId);
									if (i != _this->_ipcEndpoints.end()) {
										i->second->send(&resp, sizeof(ipc::Reply), 0);
									} else {
										LOG(ERROR) << "Error while updating device pose offset: Unknown clientId " << message.msg.dm_PoseOffset.clientId;
									}
								}
							}
							break;

						case ipc::RequestType::DeviceManipulation_PoseRotation:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.dm_PoseRotation.messageId;
								if (message.msg.dm_PoseRotation.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else if (!_trackedDeviceInfos[message.msg.dm_PoseRotation.deviceId] || !_trackedDeviceInfos[message.msg.dm_PoseRotation.deviceId]->isValid) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else {
									resp.status = ipc::ReplyStatus::Ok;
									auto device = _trackedDeviceInfos[message.msg.dm_PoseRotation.deviceId];
									if (message.msg.dm_PoseRotation.enableRotation > 0) {
										device->enablePoseRotation = message.msg.dm_PoseRotation.enableRotation == 1 ? true : false;
									}
									if (message.msg.dm_PoseRotation.rotationValid) {
										device->poseRotation = message.msg.dm_PoseRotation.rotation;
									}
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device pose offset: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									auto i = _this->_ipcEndpoints.find(message.msg.dm_PoseRotation.clientId);
									if (i != _this->_ipcEndpoints.end()) {
										i->second->send(&resp, sizeof(ipc::Reply), 0);
									} else {
										LOG(ERROR) << "Error while updating device pose offset: Unknown clientId " << message.msg.dm_PoseRotation.clientId;
									}
								}
							}
							break;

						case ipc::RequestType::DeviceManipulation_MirrorMode:
							{
								std::lock_guard<std::recursive_mutex> lock(_this->_mutex);
								ipc::Reply resp(ipc::ReplyType::GenericReply);
								resp.messageId = message.msg.dm_MirrorMode.messageId;
								if (message.msg.dm_MirrorMode.deviceId >= vr::k_unMaxTrackedDeviceCount) {
									resp.status = ipc::ReplyStatus::InvalidId;
								} else if (!_trackedDeviceInfos[message.msg.dm_MirrorMode.deviceId] || !_trackedDeviceInfos[message.msg.dm_MirrorMode.deviceId]->isValid) {
									resp.status = ipc::ReplyStatus::NotFound;
								} else {
									resp.status = ipc::ReplyStatus::Ok;
									auto device = _trackedDeviceInfos[message.msg.dm_MirrorMode.deviceId];
									device->mirrorMode = message.msg.dm_MirrorMode.mirrorMode;
									device->mirrorTarget = message.msg.dm_MirrorMode.mirrorTarget;
								}
								if (resp.status != ipc::ReplyStatus::Ok) {
									LOG(ERROR) << "Error while updating device pose offset: Error code " << (int)resp.status;
								}
								if (resp.messageId != 0) {
									auto i = _this->_ipcEndpoints.find(message.msg.dm_MirrorMode.clientId);
									if (i != _this->_ipcEndpoints.end()) {
										i->second->send(&resp, sizeof(ipc::Reply), 0);
									} else {
										LOG(ERROR) << "Error while updating device pose offset: Unknown clientId " << message.msg.dm_MirrorMode.clientId;
									}
								}
							}
							break;

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


} // end namespace driver
} // end namespace vrinputemulator
