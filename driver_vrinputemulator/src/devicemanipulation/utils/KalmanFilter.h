#pragma once

#include <openvr_driver.h>

// driver namespace
namespace vrinputemulator {
namespace driver {

// Kalman filter to filter device positions (and get their velocity at the same time)
class PosKalmanFilter {
private:
	// last a posteriori state estimate
	vr::HmdVector3d_t lastPos = { 0.0, 0.0, 0.0 };
	vr::HmdVector3d_t lastVel = { 0.0, 0.0, 0.0 };
	// last a posteriori estimate covariance matrix
	double lastCovariance[2][2] = { 0.0, 0.0, 0.0, 0.0 };
	// process noise variance
	double processNoise = 0.0;
	// observation noise variance
	double observationNoise = 0.0;
public:
	void init(const vr::HmdVector3d_t& initPos = { 0, 0, 0 }, const vr::HmdVector3d_t& initVel = { 0, 0, 0 }, const double(&initCovariance)[2][2] = { { 100.0, 0.0 },{ 0.0, 100.0 } });
	void setProcessNoise(double variance) { processNoise = variance; }
	void setObservationNoise(double variance) { observationNoise = variance; }

	void update(const vr::HmdVector3d_t& devicePos, double dt);

	const vr::HmdVector3d_t& getUpdatedPositionEstimate() { return lastPos; }
	const vr::HmdVector3d_t& getUpdatedVelocityEstimate() { return lastVel; }
};

}
}
