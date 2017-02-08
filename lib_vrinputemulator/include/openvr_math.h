#pragma once

#include <cmath>


inline vr::HmdQuaternion_t operator*(const vr::HmdQuaternion_t& lhs, const vr::HmdQuaternion_t& rhs) {
	return{
		(lhs.w * rhs.w) - (lhs.x * rhs.x) - (lhs.y * rhs.y) - (lhs.z * rhs.z),
		(lhs.w * rhs.x) + (lhs.x * rhs.w) + (lhs.y * rhs.z) - (lhs.z * rhs.y),
		(lhs.w * rhs.y) + (lhs.y * rhs.w) + (lhs.z * rhs.x) - (lhs.x * rhs.z),
		(lhs.w * rhs.z) + (lhs.z * rhs.w) + (lhs.x * rhs.y) - (lhs.y * rhs.x)
	};
}


namespace vrmath {

	inline vr::HmdQuaternion_t quatFromRotationAxis(double rot, double ux, double uy, double uz) {
		return{
			std::cos(rot / 2),
			ux * std::sin(rot / 2),
			uy * std::sin(rot / 2),
			uz * std::sin(rot / 2)
		};
	}

	inline vr::HmdQuaternion_t quatFromRotationX(double rot) {
		return{
			std::cos(rot / 2),
			std::sin(rot / 2),
			0.0f,
			0.0f
		};
	}

	inline vr::HmdQuaternion_t quatFromRotationY(double rot) {
		return{
			std::cos(rot / 2),
			0.0f,
			std::sin(rot / 2),
			0.0f
		};
	}

	inline vr::HmdQuaternion_t quatFromRotationZ(double rot) {
		return{
			std::cos(rot / 2),
			0.0f,
			0.0f,
			std::sin(rot / 2)
		};
	}

	inline vr::HmdQuaternion_t quatFromRotationYXZ(double yaw, double pitch, double roll) {
		return quatFromRotationY(yaw) * quatFromRotationX(pitch) * quatFromRotationZ(roll);
	}

	static vr::HmdQuaternion_t quatFromRotMat(const vr::HmdMatrix34_t& mat) {
		auto a = mat.m;
		vr::HmdQuaternion_t q;
		double trace = a[0][0] + a[1][1] + a[2][2];
		if (trace > 0) {
			double s = 0.5 / sqrt(trace + 1.0);
			q.w = 0.25 / s;
			q.x = (a[1][2] - a[2][1]) * s;
			q.y = (a[2][0] - a[0][2]) * s;
			q.z = (a[0][1] - a[1][0]) * s;
		} else {
			if (a[0][0] > a[1][1] && a[0][0] > a[2][2]) {
				double s = 2.0 * sqrt(1.0 + a[0][0] - a[1][1] - a[2][2]);
				q.w = (a[1][2] - a[2][1]) / s;
				q.x = 0.25 * s;
				q.y = (a[1][0] + a[0][1]) / s;
				q.z = (a[2][0] + a[0][2]) / s;
			} else if (a[1][1] > a[2][2]) {
				double s = 2.0 * sqrt(1.0 + a[1][1] - a[0][0] - a[2][2]);
				q.w = (a[2][0] - a[0][2]) / s;
				q.x = (a[1][0] + a[0][1]) / s;
				q.y = 0.25 * s;
				q.z = (a[2][1] + a[1][2]) / s;
			} else {
				double s = 2.0 * sqrt(1.0 + a[2][2] - a[0][0] - a[1][1]);
				q.w = (a[0][1] - a[1][0]) / s;
				q.x = (a[2][0] + a[0][2]) / s;
				q.y = (a[2][1] + a[1][2]) / s;
				q.z = 0.25 * s;
			}
		}
		q.x = -q.x;
		q.y = -q.y;
		q.z = -q.z;
		return q;
	}

}




