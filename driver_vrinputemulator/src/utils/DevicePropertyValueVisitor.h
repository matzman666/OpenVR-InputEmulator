#pragma once


#include <openvr_driver.h>
#include <boost/variant.hpp>

namespace vrinputemulator {
namespace driver {

class DevicePropertyValueVisitor : public boost::static_visitor<std::string> {
private:
	vr::PropertyContainerHandle_t propertyContainer;
	vr::ETrackedDeviceProperty deviceProperty;
public:
	DevicePropertyValueVisitor() = delete;
	DevicePropertyValueVisitor(vr::PropertyContainerHandle_t& propertyContainer, vr::ETrackedDeviceProperty deviceProperty)
		: propertyContainer(propertyContainer), deviceProperty(deviceProperty) {}
	template<class T>
	std::string operator()(T& i) const {
		return "Unknown value type";
	}
	template<>
	std::string operator()<int32_t>(int32_t& val) const {
		auto pError = vr::VRProperties()->SetInt32Property(propertyContainer, deviceProperty, val);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<uint64_t>(uint64_t& val) const {
		auto pError = vr::VRProperties()->SetUint64Property(propertyContainer, deviceProperty, val);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<float>(float& val) const {
		auto pError = vr::VRProperties()->SetFloatProperty(propertyContainer, deviceProperty, val);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<bool>(bool& val) const {
		auto pError = vr::VRProperties()->SetBoolProperty(propertyContainer, deviceProperty, val);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<std::string>(std::string& val) const {
		auto pError = vr::VRProperties()->SetStringProperty(propertyContainer, deviceProperty, val.c_str());
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<vr::HmdMatrix34_t>(vr::HmdMatrix34_t& val) const {
		auto pError = vr::VRProperties()->SetProperty(propertyContainer, deviceProperty, &val, sizeof(vr::HmdMatrix34_t), vr::k_unHmdMatrix34PropertyTag);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<vr::HmdMatrix44_t>(vr::HmdMatrix44_t& val) const {
		auto pError = vr::VRProperties()->SetProperty(propertyContainer, deviceProperty, &val, sizeof(vr::HmdMatrix44_t), vr::k_unHmdMatrix44PropertyTag);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<vr::HmdVector3_t>(vr::HmdVector3_t& val) const {
		auto pError = vr::VRProperties()->SetProperty(propertyContainer, deviceProperty, &val, sizeof(vr::HmdVector3_t), vr::k_unHmdVector3PropertyTag);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
	template<>
	std::string operator()<vr::HmdVector4_t>(vr::HmdVector4_t& val) const {
		auto pError = vr::VRProperties()->SetProperty(propertyContainer, deviceProperty, &val, sizeof(vr::HmdVector4_t), vr::k_unHmdVector4PropertyTag);
		if (pError == vr::TrackedProp_Success) {
			return "";
		} else {
			return std::string("OpenVR returned an error: ") + std::to_string((int)pError);
		}
	}
};


} // end namespace driver
} // end namespace vrinputemulator
