#include "KalmanFilter.h"

#include <openvr_math.h>


namespace vrinputemulator {
namespace driver {

void PosKalmanFilter::init(const vr::HmdVector3d_t& initPos, const vr::HmdVector3d_t& initVel, const double(&initCovariance)[2][2]) {
	lastPos = initPos;
	lastVel = initVel;
	lastCovariance[0][0] = initCovariance[0][0];
	lastCovariance[0][1] = initCovariance[0][1];
	lastCovariance[1][0] = initCovariance[1][0];
	lastCovariance[1][1] = initCovariance[1][1];
}

void PosKalmanFilter::update(const vr::HmdVector3d_t& devicePos, double dt) {
	// predict new position
	vr::HmdVector3d_t predictedPos = {
		lastPos.v[0] + dt * lastVel.v[0],
		lastPos.v[1] + dt * lastVel.v[1],
		lastPos.v[2] + dt * lastVel.v[2]
	};
	// no need to predict new velocity: is equal to lastVel
	// predict new covariance matrix
	double newCovariance[2][2] = {
		lastCovariance[0][0] + dt * (lastCovariance[0][1] + lastCovariance[1][0]) + dt*dt* lastCovariance[1][1] + 1.0 / 4.0 * pow(dt, 4.0) * processNoise,
		lastCovariance[0][1] + dt * lastCovariance[1][1] + 1.0 / 2.0 * pow(dt, 3.0) * processNoise,
		lastCovariance[1][0] + dt * lastCovariance[1][1] + 1.0 / 2.0 * pow(dt, 3.0) * processNoise,
		lastCovariance[1][1] + dt*dt * processNoise
	};
	// calculate innovation
	vr::HmdVector3d_t innovation = devicePos - predictedPos;
	// calculate innovation variance
	double innovationVariance = newCovariance[0][0] + observationNoise;
	// calculate kalman gain
	double gain[2];
	if (innovationVariance == 0.0) {
		gain[0] = 1;
		gain[1] = 1;
	} else {
		gain[0] = newCovariance[0][0] / innovationVariance;
		gain[1] = newCovariance[1][0] / innovationVariance;
	}
	// calculate new a posteriori position
	lastPos = lastPos + innovation * gain[0];
	// calculate new a posteriori velocity
	lastVel = lastVel + innovation * gain[1];
	// calculate new a posteriori covariance matrix
	lastCovariance[0][0] = (1 - gain[0]) * newCovariance[0][0];
	lastCovariance[0][1] = (1 - gain[0]) * newCovariance[0][1];
	lastCovariance[1][0] = newCovariance[1][0] - gain[1] * newCovariance[0][0];
	lastCovariance[1][1] = newCovariance[1][1] - gain[1] * newCovariance[0][1];
}

}
} // end namespace vrinputemulator

