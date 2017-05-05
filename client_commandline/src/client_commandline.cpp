#include "client_commandline.h"
#include <iostream>
#include <openvr.h>
#include <vrinputemulator.h>
#include <openvr_math.h>


void listDevices(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe listdevices";
		throw std::runtime_error(ss.str());
	}
	vr::EVRInitError vrInitError;
	vr::VR_Init(&vrInitError, vr::EVRApplicationType::VRApplication_Background);
	if (vrInitError != vr::VRInitError_None) {
		std::cout << "OpenVR error: " << vr::VR_GetVRInitErrorAsEnglishDescription(vrInitError) << std::endl;
		exit(2);
	}
	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
		auto deviceClass = vr::VRSystem()->GetTrackedDeviceClass(i);
		if (deviceClass != vr::TrackedDeviceClass_Invalid) {
			vr::ETrackedPropertyError pError;
			char serial[1028] = { '\0' };
			vr::VRSystem()->GetStringTrackedDeviceProperty(i, vr::Prop_SerialNumber_String, serial, 1028, &pError);
			char manufacturer[1028] = { '\0' };
			vr::VRSystem()->GetStringTrackedDeviceProperty(i, vr::Prop_ManufacturerName_String, manufacturer, 1028, &pError);
			char model[1028] = { '\0' };
			vr::VRSystem()->GetStringTrackedDeviceProperty(i, vr::Prop_ModelNumber_String, model, 1028, &pError);
			std::string deviceClassStr;
			if (deviceClass == vr::TrackedDeviceClass_HMD) {
				deviceClassStr = "HMD";
			} else if (deviceClass == vr::TrackedDeviceClass_Controller) {
				deviceClassStr = "Controller";
			} else if (deviceClass == vr::TrackedDeviceClass_GenericTracker) {
				deviceClassStr = "GenericTracker";
			} else if (deviceClass == vr::TrackedDeviceClass_TrackingReference) {
				deviceClassStr = "TrackingReference";
			} else {
				deviceClassStr = "Unknown";
			}
			std::cout << "Device " << i << " [" << deviceClassStr << "]: " << manufacturer << " - " << model 
				<< "  [" << serial << "] (connected: " << vr::VRSystem()->IsTrackedDeviceConnected(i) << ")" << std::endl;
		}
	}
	vr::VR_Shutdown();
}


void buttonEvent(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe buttonevent [press|pressandhold|unpress|touch|touchandhold|untouch] <openvrId> <buttonId> [pressTime]";
		throw std::runtime_error(ss.str());
	} else if (argc < 5) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	bool noHold = false;
	vrinputemulator::ButtonEventType eventType;
	if (std::strcmp(argv[2], "press") == 0) {
		eventType = vrinputemulator::ButtonEventType::ButtonPressed;
		noHold = true;
	} else if (std::strcmp(argv[2], "pressandhold") == 0) {
		eventType = vrinputemulator::ButtonEventType::ButtonPressed;
	} else if (std::strcmp(argv[2], "unpress") == 0) {
		eventType = vrinputemulator::ButtonEventType::ButtonUnpressed;
	} else if (std::strcmp(argv[2], "touch") == 0) {
		eventType = vrinputemulator::ButtonEventType::ButtonTouched;
		noHold = true;
	} else if (std::strcmp(argv[2], "touchandhold") == 0) {
		eventType = vrinputemulator::ButtonEventType::ButtonTouched;
	} else if (std::strcmp(argv[2], "untouch") == 0) {
		eventType = vrinputemulator::ButtonEventType::ButtonUntouched;
	} else {
		throw std::runtime_error( std::string("Error: Unknown button event type ") + std::string(argv[2]));
	}
	uint32_t holdTime = 50;
	if (noHold) {
		if (argc > 5) {
			holdTime = std::atoi(argv[5]);
		}
	}
	uint32_t deviceId = std::atoi(argv[3]);
	vr::EVRButtonId buttonId = (vr::EVRButtonId)std::atoi(argv[4]);
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	inputEmulator.openvrButtonEvent(eventType, deviceId, buttonId, 0.0);
	if (noHold) {
		std::this_thread::sleep_for(std::chrono::milliseconds(holdTime));
		if (eventType == vrinputemulator::ButtonEventType::ButtonPressed) {
			eventType = vrinputemulator::ButtonEventType::ButtonUnpressed;
		} else {
			eventType = vrinputemulator::ButtonEventType::ButtonUntouched;
		}
		inputEmulator.openvrButtonEvent(eventType, deviceId, buttonId, 0.0);
	}
}



void axisEvent(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe axisevent <openvrId> <axisId> <x> <y>";
		throw std::runtime_error(ss.str());
	} else if (argc < 6) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	uint32_t deviceId = std::atoi(argv[2]);
	uint32_t axisId = std::atoi(argv[3]);
	vr::VRControllerAxis_t axisState;
	axisState.x = (float)std::atof(argv[4]);
	axisState.y = (float)std::atof(argv[5]);
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	inputEmulator.openvrAxisEvent(deviceId, axisId, axisState);
}

void proximitySensorEvent(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe proximitysensor <openvrId> [0|1]";
		throw std::runtime_error(ss.str());
	} else if (argc < 4) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	uint32_t deviceId = std::atoi(argv[2]);
	bool triggered = std::atoi(argv[3]) != 0;
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	inputEmulator.openvrProximitySensorEvent(deviceId, triggered);
}


void listVirtual(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe listvirtual";
		throw std::runtime_error(ss.str());
	}
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	auto deviceCount = inputEmulator.getVirtualDeviceCount();
	for (unsigned i = 0; i < deviceCount; ++i) {
		auto deviceInfo = inputEmulator.getVirtualDeviceInfo(i);
		std::string deviceType;
		switch (deviceInfo.deviceType) {
			case vrinputemulator::VirtualDeviceType::TrackedController:
				deviceType = "TrackedController";
				break;
			default:
				deviceType = "Unknown";
				break;
		}
		std::string openVRIdStr;
		if (deviceInfo.openvrDeviceId == vr::k_unTrackedDeviceIndexInvalid) {
			openVRIdStr = "<none>";
		} else {
			openVRIdStr = std::to_string(deviceInfo.openvrDeviceId);
		}
		std::cout << "Virtual Device " << i << " [" << deviceType << "]: Serial " << deviceInfo.deviceSerial << ", OpenVR Id " << openVRIdStr << std::endl;
	}
}


void addTrackedController(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe addcontroller <serialnumber>";
		throw std::runtime_error(ss.str());
	} else if (argc < 3) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	std::cout << inputEmulator.addVirtualDevice(vrinputemulator::VirtualDeviceType::TrackedController, argv[2], false);
}

void publishTrackedDevice(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe publishdevice <virtualId>";
		throw std::runtime_error(ss.str());
	} else if (argc < 3) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	uint32_t deviceId = std::atoi(argv[2]);
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	inputEmulator.publishVirtualDevice(deviceId);
}

void setDeviceProperty(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe setdeviceproperty <virtualId> <property> [int32|uint64|float|bool|string] <value>";
		throw std::runtime_error(ss.str());
	} else if (argc < 6) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	uint32_t deviceId = std::atoi(argv[2]);
	vr::ETrackedDeviceProperty deviceProperty = (vr::ETrackedDeviceProperty)std::atoi(argv[3]);
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	if (std::strcmp(argv[4], "int32") == 0) {
		inputEmulator.setVirtualDeviceProperty(deviceId, deviceProperty, (int32_t)std::atoi(argv[5]));
	} else if (std::strcmp(argv[4], "uint64") == 0) {
		inputEmulator.setVirtualDeviceProperty(deviceId, deviceProperty, (uint64_t)std::atoll(argv[5]));
	} else if (std::strcmp(argv[4], "float") == 0) {
		inputEmulator.setVirtualDeviceProperty(deviceId, deviceProperty, (float)std::atof(argv[5]));
	} else if (std::strcmp(argv[4], "bool") == 0) {
		inputEmulator.setVirtualDeviceProperty(deviceId, deviceProperty, std::atoi(argv[5]) != 0);
	} else if (std::strcmp(argv[4], "string") == 0) {
		inputEmulator.setVirtualDeviceProperty(deviceId, deviceProperty, argv[5]);
	} else {
		throw std::runtime_error("Unknown value type.");
	}
}

void getDeviceProperty(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe getdeviceproperty <openvrId> scan" << std::endl
			<< "       client_commandline.exe getdeviceproperty <openvrId> <property> [int32|uint64|float|bool|string]";
		throw std::runtime_error(ss.str());
	} else if (argc < 4) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	uint32_t deviceId = std::atoi(argv[2]);
	vr::EVRInitError vrInitError;
	vr::VR_Init(&vrInitError, vr::EVRApplicationType::VRApplication_Background);
	if (vrInitError != vr::VRInitError_None) {
		std::cout << "OpenVR error: " << vr::VR_GetVRInitErrorAsEnglishDescription(vrInitError) << std::endl;
		exit(2);
	}
	if (std::strcmp(argv[3], "scan") == 0) {
		for (int p = 1; p < 20000; p++) {
			vr::ETrackedPropertyError error;
			auto valueInt32 = vr::VRSystem()->GetInt32TrackedDeviceProperty(deviceId, (vr::ETrackedDeviceProperty)p, &error);
			if (error == vr::TrackedProp_Success) {
				std::cout << p << "\tint32\t" << valueInt32 << std::endl;
				continue;
			}
			auto valueUint64 = vr::VRSystem()->GetUint64TrackedDeviceProperty(deviceId, (vr::ETrackedDeviceProperty)p, &error);
			if (error == vr::TrackedProp_Success) {
				std::cout << p << "\tuint64\t" << valueUint64 << std::endl;
				continue;
			}
			auto valueBool = vr::VRSystem()->GetBoolTrackedDeviceProperty(deviceId, (vr::ETrackedDeviceProperty)p, &error);
			if (error == vr::TrackedProp_Success) {
				std::cout << p << "\tbool\t" << valueBool << std::endl;
				continue;
			}
			auto valueFloat = vr::VRSystem()->GetFloatTrackedDeviceProperty(deviceId, (vr::ETrackedDeviceProperty)p, &error);
			if (error == vr::TrackedProp_Success) {
				std::cout << p << "\tfloat\t" << valueFloat << std::endl;
				continue;
			}
			char buffer[1024] = { '\0' };
			vr::VRSystem()->GetStringTrackedDeviceProperty(deviceId, (vr::ETrackedDeviceProperty)p, buffer, 1024, &error);
			if (error == vr::TrackedProp_Success) {
				std::cout << p << "\tstring\t" << buffer << std::endl;
				continue;
			}
			auto valueMatrix34 = vr::VRSystem()->GetMatrix34TrackedDeviceProperty(deviceId, (vr::ETrackedDeviceProperty)p, &error);
			if (error == vr::TrackedProp_Success) {
				std::cout << p << "\tmatrix34\t{";
				for (int i = 0; i < 3; ++i) {
					std::cout << "{";
					bool isFirst = true;
					for (int j = 0; j < 4; ++j) {
						if (isFirst) {
							isFirst = false;
						} else {
							std::cout << ", ";
						}
						std::cout << valueMatrix34.m[i][j];
					}
					std::cout << "}";
				}
				std::cout << "}" << std::endl;
				continue;
			}
		}
	} else {
		if (argc < 5) {
			throw std::runtime_error("Error: Too few arguments.");
		}
		vr::ETrackedDeviceProperty deviceProperty = (vr::ETrackedDeviceProperty)std::atoi(argv[3]);
		if (std::strcmp(argv[4], "int32") == 0) {
			vr::ETrackedPropertyError error;
			auto value = vr::VRSystem()->GetInt32TrackedDeviceProperty(deviceId, deviceProperty, &error);
			if (error == vr::TrackedProp_Success) {
				std::cout << value;
			} else {
				std::stringstream ss;
				ss << "Could not get device property: " << vr::VRSystem()->GetPropErrorNameFromEnum(error);
				throw std::runtime_error(ss.str());
			}
		} else if (std::strcmp(argv[4], "uint64") == 0) {
			vr::ETrackedPropertyError error;
			auto value = vr::VRSystem()->GetUint64TrackedDeviceProperty(deviceId, deviceProperty, &error);
			if (error == vr::TrackedProp_Success) {
				std::cout << value;
			} else {
				std::stringstream ss;
				ss << "Could not get device property: " << vr::VRSystem()->GetPropErrorNameFromEnum(error);
				throw std::runtime_error(ss.str());
			}
		} else if (std::strcmp(argv[4], "float") == 0) {
			vr::ETrackedPropertyError error;
			auto value = vr::VRSystem()->GetFloatTrackedDeviceProperty(deviceId, deviceProperty, &error);
			if (error == vr::TrackedProp_Success) {
				std::cout << value;
			} else {
				std::stringstream ss;
				ss << "Could not get device property: " << vr::VRSystem()->GetPropErrorNameFromEnum(error);
				throw std::runtime_error(ss.str());
			}
		} else if (std::strcmp(argv[4], "bool") == 0) {
			vr::ETrackedPropertyError error;
			auto value = vr::VRSystem()->GetBoolTrackedDeviceProperty(deviceId, deviceProperty, &error);
			if (error == vr::TrackedProp_Success) {
				std::cout << (value ? "true" : "false");
			} else {
				std::stringstream ss;
				ss << "Could not get device property: " << vr::VRSystem()->GetPropErrorNameFromEnum(error);
				throw std::runtime_error(ss.str());
			}
		} else if (std::strcmp(argv[4], "string") == 0) {
			vr::ETrackedPropertyError error;
			char buffer[1024] = { '\0' };
			vr::VRSystem()->GetStringTrackedDeviceProperty(deviceId, deviceProperty, buffer, 1024, &error);
			if (error == vr::TrackedProp_Success) {
				std::cout << buffer;
			} else {
				std::stringstream ss;
				ss << "Could not get device property: " << vr::VRSystem()->GetPropErrorNameFromEnum(error);
				throw std::runtime_error(ss.str());
			}
		} else if (std::strcmp(argv[4], "matrix34") == 0) {
			vr::ETrackedPropertyError error;
			auto value = vr::VRSystem()->GetMatrix34TrackedDeviceProperty(deviceId, deviceProperty, &error);
			if (error == vr::TrackedProp_Success) {
				std::cout << "{";
				for (int i = 0; i < 3; ++i) {
					std::cout << "{";
					bool isFirst = true;
					for (int j = 0; j < 4; ++j) {
						if (isFirst) {
							isFirst = false;
						} else {
							std::cout << ", ";
						}
						std::cout << value.m[i][j];
					}
					std::cout << "}";
				}
				std::cout << "}";
			} else {
				std::stringstream ss;
				ss << "Could not get device property: " << vr::VRSystem()->GetPropErrorNameFromEnum(error);
				throw std::runtime_error(ss.str());
			}
		} else {
			throw std::runtime_error("Unknown value type.");
		}
	}
	vr::VR_Shutdown();
}


void removeDeviceProperty(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe removedeviceproperty <virtualId> <property>";
		throw std::runtime_error(ss.str());
	} else if (argc < 4) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	uint32_t deviceId = std::atoi(argv[2]);
	vr::ETrackedDeviceProperty deviceProperty = (vr::ETrackedDeviceProperty)std::atoi(argv[3]);
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	inputEmulator.removeVirtualDeviceProperty(deviceId, deviceProperty);
}


void setDeviceConnection(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe setdeviceconnection <virtualId> [0|1]";
		throw std::runtime_error(ss.str());
	} else if (argc < 4) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	uint32_t deviceId = std::atoi(argv[2]);
	bool connected = std::atoi(argv[3]) != 0;
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	auto pose = inputEmulator.getVirtualDevicePose(deviceId);
	if (pose.deviceIsConnected != connected) {
		pose.deviceIsConnected = connected;
		pose.poseIsValid = connected;
		inputEmulator.setVirtualDevicePose(deviceId, pose);
	}
}


void setDevicePosition(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe setdeviceposition <virtualId> <x> <y> <z>";
		throw std::runtime_error(ss.str());
	} else if (argc < 6) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	uint32_t deviceId = std::atoi(argv[2]);
	float x = (float)std::atof(argv[3]);
	float y = (float)std::atof(argv[4]);
	float z = (float)std::atof(argv[5]);
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	auto pose = inputEmulator.getVirtualDevicePose(deviceId);
	pose.vecPosition[0] = x;
	pose.vecPosition[1] = y;
	pose.vecPosition[2] = z;
	pose.poseIsValid = true;
	pose.result = vr::TrackingResult_Running_OK;
	inputEmulator.setVirtualDevicePose(deviceId, pose);
}


void setDeviceRotation(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe setdevicerotation <virtualId> <yaw> <pitch> <roll>";
		throw std::runtime_error(ss.str());
	} else if (argc < 6) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	uint32_t deviceId = std::atoi(argv[2]);
	float yaw = (float)std::atof(argv[3]);
	float pitch = (float)std::atof(argv[4]);
	float roll = (float)std::atof(argv[5]);
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	auto pose = inputEmulator.getVirtualDevicePose(deviceId);
	pose.qRotation = vrmath::quaternionFromYawPitchRoll(yaw, pitch, roll);
	pose.poseIsValid = true;
	pose.result = vr::TrackingResult_Running_OK;
	inputEmulator.setVirtualDevicePose(deviceId, pose);
}

void deviceButtonMapping(int argc, const char * argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss <<  "Usage: client_commandline.exe devicebuttonmapping <openvrId> [enable|disable]" << std::endl
			<< "       client_commandline.exe devicebuttonmapping <openvrId> add <buttonId> <mappedButtonId>" << std::endl
			<< "       client_commandline.exe devicebuttonmapping <openvrId> remove [<buttonId>|all]";
		throw std::runtime_error(ss.str());
	} else if (argc < 4) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	uint32_t deviceId = std::atoi(argv[2]);
	uint32_t enable = 0;
	uint32_t operation = 0;
	uint32_t op1 = 0;
	uint32_t op2 = 0;
	if (std::strcmp(argv[3], "enable") == 0) {
		enable = 1;
	} else if (std::strcmp(argv[3], "disable") == 0) {
		enable = 2;
	} else if (std::strcmp(argv[3], "add") == 0) {
		if (argc < 6) {
			throw std::runtime_error("Error: Too few arguments.");
		}
		operation = 1;
		op1 = std::atoi(argv[4]);
		op2 = std::atoi(argv[5]);
	} else if (std::strcmp(argv[3], "remove") == 0) {
		if (argc < 5) {
			throw std::runtime_error("Error: Too few arguments.");
		}
		if (std::strcmp(argv[4], "all") == 0) {
			operation = 3;
		} else {
			operation = 2;
			op1 = std::atoi(argv[4]);
		}

	} else {
		throw std::runtime_error("Error: Unknown button mapping command");
	}
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	if (enable > 0) {
		inputEmulator.enableDeviceButtonMapping(deviceId, enable == 1 ? true : false);
	} else if (operation == 1) {
		inputEmulator.addDeviceButtonMapping(deviceId, (vr::EVRButtonId)op1, (vr::EVRButtonId)op2);
	} else if (operation == 2) {
		inputEmulator.removeDeviceButtonMapping(deviceId, (vr::EVRButtonId)op1);
	} else if (operation == 3) {
		inputEmulator.removeAllDeviceButtonMappings(deviceId);
	}
}

void deviceOffsets(int argc, const char * argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe deviceoffsets <openvrId> [enable|disable]" << std::endl
			<< "       client_commandline.exe deviceoffsets <openvrId> set worldPosOffset <x> <y> <z>" << std::endl
			<< "       client_commandline.exe deviceoffsets <openvrId> set worldRotOffset <yaw> <pitch> <roll>" << std::endl
			<< "       client_commandline.exe deviceoffsets <openvrId> set driverPosOffset <x> <y> <z>" << std::endl
			<< "       client_commandline.exe deviceoffsets <openvrId> set driverRotOffset <yaw> <pitch> <roll>" << std::endl
			<< "       client_commandline.exe deviceoffsets <openvrId> set devicePosOffset <x> <y> <z>" << std::endl
			<< "       client_commandline.exe deviceoffsets <openvrId> set deviceRotOffset <yaw> <pitch> <roll>";
		throw std::runtime_error(ss.str());
	} else if (argc < 4) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	uint32_t deviceId = std::atoi(argv[2]);
	uint32_t enable = 0;
	bool offsetValid = false;
	int universe = 0;
	int type = 0;
	double offset[3];
	if (std::strcmp(argv[3], "enable") == 0) {
		enable = 1;
	} else if (std::strcmp(argv[3], "disable") == 0) {
		enable = 2;
	} else if (std::strcmp(argv[3], "set") == 0) {
		if (argc < 8) {
			throw std::runtime_error("Error: Too few arguments.");
		}
		if (std::strcmp(argv[3], "worldPosOffset") == 0) {
			universe = 1;
			type = 2;
		} else if (std::strcmp(argv[4], "worldRotOffset") == 0) {
			universe = 1;
			type = 1;
		} else if (std::strcmp(argv[4], "driverPosOffset") == 0) {
			universe = 2;
			type = 2;
		} else if (std::strcmp(argv[4], "driverRotOffset") == 0) {
			universe = 2;
			type = 1;
		} else if (std::strcmp(argv[4], "deviceRotOffset") == 0) {
			universe = 3;
			type = 1;
		} else if (std::strcmp(argv[4], "devicePosOffset") == 0) {
			universe = 3;
			type = 2;
		} else {
			throw std::runtime_error("Error: Unknown argument.");
		}
		offset[0] = std::atof(argv[5]);
		offset[1] = std::atof(argv[6]);
		offset[2] = std::atof(argv[7]);
		offsetValid = true;
	} else {
		throw std::runtime_error("Error: Unknown pose offset command");
	}
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	if (enable > 0) {
		inputEmulator.enableDeviceOffsets(deviceId, enable == 1 ? true : false);
	} else if (offsetValid) {
		if (type == 1) {
			vr::HmdQuaternion_t rot = vrmath::quaternionFromYawPitchRoll(offset[0], offset[1], offset[2]);
			switch (universe) {
			case 1:
				inputEmulator.setWorldFromDriverRotationOffset(deviceId, rot);
				break;
			case 2:
				inputEmulator.setDriverFromHeadRotationOffset(deviceId, rot);
				break;
			case 3:
				inputEmulator.setDriverRotationOffset(deviceId, rot);
				break;
			default:
				throw std::runtime_error("Error: Unknown universe");
			}
		} else if (type == 2) {
			vr::HmdVector3d_t pos = {offset[0], offset[1], offset[2]};
			switch (universe) {
			case 1:
				inputEmulator.setWorldFromDriverTranslationOffset(deviceId, pos);
				break;
			case 2:
				inputEmulator.setDriverFromHeadTranslationOffset(deviceId, pos);
				break;
			case 3:
				inputEmulator.setDriverTranslationOffset(deviceId, pos);
				break;
			default:
				throw std::runtime_error("Error: Unknown universe");
			}
		} else {
			throw std::runtime_error("Error: Unknown type");
		}
	}
}

void deviceModes(int argc, const char * argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe devicemirrormode <openvrId> off" << std::endl
			<< "       client_commandline.exe devicemirrormode <openvrId> [mirror|redirect] <targetOpenvrId>";
		throw std::runtime_error(ss.str());
	} else if (argc < 4) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	/*uint32_t deviceId = std::atoi(argv[2]);
	uint32_t mode;
	uint32_t target = vr::k_unTrackedDeviceIndexInvalid;
	if (std::strcmp(argv[3], "off") == 0) {
		mode = 0;
	} else if (std::strcmp(argv[3], "mirror") == 0) {
		mode = 1;
	} else if (std::strcmp(argv[3], "redirect") == 0) {
		mode = 2;
	}
	if (mode > 0) {
		if (argc < 5) {
			throw std::runtime_error("Error: Too few arguments.");
		}
		target = std::atoi(argv[4]);
	}
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	inputEmulator.setDeviceMirrorMode(deviceId, mode, target);*/
}




void benchmarkIPC(int argc, const char* argv[]) {
	if (argc > 2 && std::strcmp(argv[2], "help") == 0) {
		std::stringstream ss;
		ss << "Usage: client_commandline.exe benchmarkipc [all|roundtrip|throughput1|throughput2|throughput]";
		throw std::runtime_error(ss.str());
	} else if (argc < 3) {
		throw std::runtime_error("Error: Too few arguments.");
	}
	uint32_t benchmarkMask = 0;
	if (std::strcmp(argv[2], "all") == 0) {
		benchmarkMask = 0xFFFFFFFF;
	} else if (std::strcmp(argv[2], "roundtrip") == 0) {
		benchmarkMask = 1;
	} else if (std::strcmp(argv[2], "throughput1") == 0) {
		benchmarkMask = 1 << 1;
	} else if (std::strcmp(argv[2], "throughput2") == 0) {
		benchmarkMask = 1 << 2;
	} else if (std::strcmp(argv[2], "throughput") == 0) {
		benchmarkMask = (1 << 2) | (1 << 1);
	} else {
		throw std::runtime_error("Error: Unknown benchmark");
	}
	unsigned loopCounterMax = 1000;
	if (argc > 3) {
		loopCounterMax = std::atoi(argv[3]);
	}
	std::cout << "Message count: " << loopCounterMax << std::endl;
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	if (benchmarkMask & 1) {
		auto startTime = std::chrono::system_clock::now();
		for (unsigned i = 0; i < loopCounterMax; ++i) {
			inputEmulator.ping();
		}
		auto stopTime = std::chrono::system_clock::now();
		auto timeDiff = stopTime - startTime;
		double timeMillis = (double)std::chrono::duration_cast <std::chrono::milliseconds>(timeDiff).count();
		std::cout << "Average IPC round-trip time: " << timeMillis / (double)loopCounterMax << " ms (total time: " << timeMillis << " ms)" << std::endl;
		std::cout << "Average IPC round-trip messages/s: " << 1000.0 * (double)loopCounterMax / timeMillis << " msg/s" << std::endl;
	}
	if (benchmarkMask & (1 << 1)) {
		auto startTime = std::chrono::system_clock::now();
		for (unsigned i = 0; i < loopCounterMax; ++i) {
			inputEmulator.ping(false, true);
		}
		inputEmulator.ping(); // wait till queue is empty
		auto stopTime = std::chrono::system_clock::now();
		auto timeDiff = stopTime - startTime;
		double timeMillis = (double)std::chrono::duration_cast <std::chrono::milliseconds>(timeDiff).count();
		std::cout << "Average IPC two-way messages/s: " << 1000.0 * (double)loopCounterMax / timeMillis << " msg/s (total time: " << timeMillis << " ms)" << std::endl;
	}
	if (benchmarkMask & (1 << 2)) {
		auto startTime = std::chrono::system_clock::now();
		for (unsigned i = 0; i < loopCounterMax; ++i) {
			inputEmulator.ping(false, false);
		}
		inputEmulator.ping(); // wait till queue is empty
		auto stopTime = std::chrono::system_clock::now();
		auto timeDiff = stopTime - startTime;
		double timeMillis = (double)std::chrono::duration_cast <std::chrono::milliseconds>(timeDiff).count();
		std::cout << "Average IPC one-way messages/s: " << 1000.0 * (double)loopCounterMax / timeMillis << " msg/s (total time: " << timeMillis << " ms)" << std::endl;
	}
	std::cout << "IPC request size: " << sizeof(vrinputemulator::ipc::Request) << " bytes" << std::endl;
	std::cout << "IPC reply size: " << sizeof(vrinputemulator::ipc::Reply) << " bytes" << std::endl;
}
