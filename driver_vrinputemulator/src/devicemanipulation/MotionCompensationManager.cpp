#include "MotionCompensationManager.h"

#include "DeviceManipulationHandle.h"
#include "../driver/ServerDriver.h"


// driver namespace
namespace vrinputemulator {
namespace driver {


void MotionCompensationManager::enableMotionCompensation(bool enable) {
	_motionCompensationZeroRefTimeout = 0;
	_motionCompensationZeroPoseValid = false;
	_motionCompensationRefPoseValid = false;
	_motionCompensationEnabled = enable;
	if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::KalmanFilter) {
		m_parent->executeCodeForEachDeviceManipulationHandle([](DeviceManipulationHandle* handle) {
			handle->setLastPoseTime(-1);
		});
	}
}

void MotionCompensationManager::setMotionCompensationRefDevice(DeviceManipulationHandle* device) {
	_motionCompensationRefDevice = device;
}

DeviceManipulationHandle* MotionCompensationManager::getMotionCompensationRefDevice() {
	return _motionCompensationRefDevice;
}

void MotionCompensationManager::setMotionCompensationVelAccMode(MotionCompensationVelAccMode velAccMode) {
	if (_motionCompensationVelAccMode != velAccMode) {
		_motionCompensationRefVelAccValid = false;
		m_parent->executeCodeForEachDeviceManipulationHandle([this](DeviceManipulationHandle* handle) {
			handle->setLastPoseTime(-1);
			handle->kalmanFilter().setProcessNoise(m_motionCompensationKalmanProcessVariance);
			handle->kalmanFilter().setObservationNoise(m_motionCompensationKalmanObservationVariance);
			handle->velMovingAverage().resize(m_motionCompensationMovingAverageWindow);
		});
		_motionCompensationVelAccMode = velAccMode;
	}
}

void MotionCompensationManager::setMotionCompensationKalmanProcessVariance(double variance) {
	m_motionCompensationKalmanProcessVariance = variance;
	if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::KalmanFilter) {
		m_parent->executeCodeForEachDeviceManipulationHandle([variance](DeviceManipulationHandle* handle) {
			handle->kalmanFilter().setProcessNoise(variance);
		});
	}
}

void MotionCompensationManager::setMotionCompensationKalmanObservationVariance(double variance) {
	m_motionCompensationKalmanObservationVariance = variance;
	if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::KalmanFilter) {
		m_parent->executeCodeForEachDeviceManipulationHandle([variance](DeviceManipulationHandle* handle) {
			handle->kalmanFilter().setObservationNoise(variance);
		});
	}
}

void MotionCompensationManager::setMotionCompensationMovingAverageWindow(unsigned window) {
	m_motionCompensationMovingAverageWindow = window;
	if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::LinearApproximation) {
		m_parent->executeCodeForEachDeviceManipulationHandle([window](DeviceManipulationHandle* handle) {
			handle->velMovingAverage().resize(window);
		});
	}
}

void MotionCompensationManager::_disableMotionCompensationOnAllDevices() {
	m_parent->executeCodeForEachDeviceManipulationHandle([](DeviceManipulationHandle* handle) {
		if (handle->deviceMode() == 5) {
			handle->setDefaultMode();
		}
	});
}

bool MotionCompensationManager::_isMotionCompensationZeroPoseValid() {
	return _motionCompensationZeroPoseValid;
}

void MotionCompensationManager::_setMotionCompensationZeroPose(const vr::DriverPose_t& pose) {
	// convert pose from driver space to app space
	auto tmpConj = vrmath::quaternionConjugate(pose.qWorldFromDriverRotation);
	_motionCompensationZeroPos = vrmath::quaternionRotateVector(pose.qWorldFromDriverRotation, tmpConj, pose.vecPosition, true) - pose.vecWorldFromDriverTranslation;
	_motionCompensationZeroRot = tmpConj * pose.qRotation;

	_motionCompensationZeroPoseValid = true;
}

void MotionCompensationManager::_updateMotionCompensationRefPose(const vr::DriverPose_t& pose) {
	// convert pose from driver space to app space
	auto tmpConj = vrmath::quaternionConjugate(pose.qWorldFromDriverRotation);
	_motionCompensationRefPos = vrmath::quaternionRotateVector(pose.qWorldFromDriverRotation, tmpConj, pose.vecPosition, true) - pose.vecWorldFromDriverTranslation;
	auto poseWorldRot = tmpConj * pose.qRotation;

	// calculate orientation difference and its inverse
	_motionCompensationRotDiff = poseWorldRot * vrmath::quaternionConjugate(_motionCompensationZeroRot);
	_motionCompensationRotDiffInv = vrmath::quaternionConjugate(_motionCompensationRotDiff);

	// Convert velocity and acceleration values into app space and undo device rotation
	if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::SubstractMotionRef) {
		auto tmpRot = tmpConj * vrmath::quaternionConjugate(pose.qRotation);
		auto tmpRotInv = vrmath::quaternionConjugate(tmpRot);
		_motionCompensationRefPosVel = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, { pose.vecVelocity[0], pose.vecVelocity[1], pose.vecVelocity[2] });
		_motionCompensationRefPosAcc = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, { pose.vecAcceleration[0], pose.vecAcceleration[1], pose.vecAcceleration[2] });
		_motionCompensationRefRotVel = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, { pose.vecAngularVelocity[0], pose.vecAngularVelocity[1], pose.vecAngularVelocity[2] });
		_motionCompensationRefRotAcc = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, { pose.vecAngularAcceleration[0], pose.vecAngularAcceleration[1], pose.vecAngularAcceleration[2] });
		_motionCompensationRefVelAccValid = true;
	}

	_motionCompensationRefPoseValid = true;
}

bool MotionCompensationManager::_applyMotionCompensation(vr::DriverPose_t& pose, DeviceManipulationHandle* deviceInfo) {
	if (_motionCompensationEnabled && _motionCompensationZeroPoseValid && _motionCompensationRefPoseValid) {
		// convert pose from driver space to app space
		vr::HmdQuaternion_t tmpConj = vrmath::quaternionConjugate(pose.qWorldFromDriverRotation);
		auto poseWorldPos = vrmath::quaternionRotateVector(pose.qWorldFromDriverRotation, tmpConj, pose.vecPosition, true) - pose.vecWorldFromDriverTranslation;
		auto poseWorldRot = tmpConj * pose.qRotation;

		// do motion compensation
		auto compensatedPoseWorldPos = _motionCompensationZeroPos + vrmath::quaternionRotateVector(_motionCompensationRotDiff, _motionCompensationRotDiffInv, poseWorldPos - _motionCompensationRefPos, true);
		auto compensatedPoseWorldRot = _motionCompensationRotDiffInv * poseWorldRot;

		// Velocity / Acceleration Compensation
		vr::HmdVector3d_t compensatedPoseWorldVel;
		bool compensatedPoseWorldVelValid = false;
		bool setVelToZero = false;
		bool setAccToZero = false;
		bool setAngVelToZero = false;
		bool setAngAccToZero = false;

		auto now = std::chrono::duration_cast <std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::SetZero) {
			setVelToZero = true;
			setAccToZero = true;
			setAngVelToZero = true;
			setAngAccToZero = true;

		} else if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::SubstractMotionRef) {
			// We translate the motion ref vel/acc values into driver space and directly substract them
			if (_motionCompensationRefVelAccValid) {
				auto tmpRot = pose.qWorldFromDriverRotation * pose.qRotation;
				auto tmpRotInv = vrmath::quaternionConjugate(tmpRot);
				auto tmpPosVel = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, _motionCompensationRefPosVel);
				pose.vecVelocity[0] -= tmpPosVel.v[0];
				pose.vecVelocity[1] -= tmpPosVel.v[1];
				pose.vecVelocity[2] -= tmpPosVel.v[2];
				auto tmpPosAcc = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, _motionCompensationRefPosAcc);
				pose.vecAcceleration[0] -= tmpPosAcc.v[0];
				pose.vecAcceleration[1] -= tmpPosAcc.v[1];
				pose.vecAcceleration[2] -= tmpPosAcc.v[2];
				auto tmpRotVel = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, _motionCompensationRefRotVel);
				pose.vecAngularVelocity[0] -= tmpRotVel.v[0];
				pose.vecAngularVelocity[1] -= tmpRotVel.v[1];
				pose.vecAngularVelocity[2] -= tmpRotVel.v[2];
				auto tmpRotAcc = vrmath::quaternionRotateVector(tmpRot, tmpRotInv, _motionCompensationRefRotAcc);
				pose.vecAngularAcceleration[0] -= tmpRotAcc.v[0];
				pose.vecAngularAcceleration[1] -= tmpRotAcc.v[1];
				pose.vecAngularAcceleration[2] -= tmpRotAcc.v[2];
			}

		} else if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::KalmanFilter) {
			// The Kalman filter uses app space coordinates
			auto lastTime = deviceInfo->getLastPoseTime();
			if (lastTime >= 0.0) {
				double tdiff = ((double)(now - lastTime) / 1.0E6) + (pose.poseTimeOffset - deviceInfo->getLastPoseTimeOffset());
				if (tdiff < 0.0001) { // Sometimes we get a very small or even negative time difference between current and last pose
										// In this case we just take the velocities and accelerations from last time
					auto& lastPose = deviceInfo->lastDriverPose();
					pose.vecVelocity[0] = lastPose.vecVelocity[0];
					pose.vecVelocity[1] = lastPose.vecVelocity[1];
					pose.vecVelocity[2] = lastPose.vecVelocity[2];
					pose.vecAcceleration[0] = lastPose.vecAcceleration[0];
					pose.vecAcceleration[1] = lastPose.vecAcceleration[1];
					pose.vecAcceleration[2] = lastPose.vecAcceleration[2];
					pose.vecAngularVelocity[0] = lastPose.vecAngularVelocity[0];
					pose.vecAngularVelocity[1] = lastPose.vecAngularVelocity[1];
					pose.vecAngularVelocity[2] = lastPose.vecAngularVelocity[2];
					pose.vecAngularAcceleration[0] = lastPose.vecAngularAcceleration[0];
					pose.vecAngularAcceleration[1] = lastPose.vecAngularAcceleration[1];
					pose.vecAngularAcceleration[2] = lastPose.vecAngularAcceleration[2];
				} else {
					deviceInfo->kalmanFilter().update(compensatedPoseWorldPos, tdiff);
					//compensatedPoseWorldPos = deviceInfo->kalmanFilter().getUpdatedPositionEstimate(); // Better to use the original values
					compensatedPoseWorldVel = deviceInfo->kalmanFilter().getUpdatedVelocityEstimate();
					compensatedPoseWorldVelValid = true;
					// Kalman filter only gives us velocity, so set the rest to zero
					setAccToZero = true;
					setAngVelToZero = true;
					setAngAccToZero = true;
				}
			} else {
				deviceInfo->kalmanFilter().init(
					compensatedPoseWorldPos,
					{ 0.0, 0.0, 0.0 },
					{ { 0.0, 0.0 },{ 0.0, 0.0 } }
				);
				deviceInfo->kalmanFilter().setProcessNoise(m_motionCompensationKalmanProcessVariance);
				deviceInfo->kalmanFilter().setObservationNoise(m_motionCompensationKalmanObservationVariance);
				// Kalman Filter is not ready yet, so set everything to zero
				setVelToZero = true;
				setAccToZero = true;
				setAngVelToZero = true;
				setAngAccToZero = true;
			}

		} else if (_motionCompensationVelAccMode == MotionCompensationVelAccMode::LinearApproximation) {
			// Linear approximation uses driver space coordinates
			if (deviceInfo->lastDriverPoseValid()) {
				auto& lastPose = deviceInfo->lastDriverPose();
				double tdiff = ((double)(now - deviceInfo->getLastPoseTime()) / 1.0E6) + (pose.poseTimeOffset - lastPose.poseTimeOffset);
				if (tdiff < 0.0001) { // Sometimes we get a very small or even negative time difference between current and last pose
										// In this case we just take the velocities and accelerations from last time
					pose.vecVelocity[0] = lastPose.vecVelocity[0];
					pose.vecVelocity[1] = lastPose.vecVelocity[1];
					pose.vecVelocity[2] = lastPose.vecVelocity[2];
					pose.vecAcceleration[0] = lastPose.vecAcceleration[0];
					pose.vecAcceleration[1] = lastPose.vecAcceleration[1];
					pose.vecAcceleration[2] = lastPose.vecAcceleration[2];
					pose.vecAngularVelocity[0] = lastPose.vecAngularVelocity[0];
					pose.vecAngularVelocity[1] = lastPose.vecAngularVelocity[1];
					pose.vecAngularVelocity[2] = lastPose.vecAngularVelocity[2];
					pose.vecAngularAcceleration[0] = lastPose.vecAngularAcceleration[0];
					pose.vecAngularAcceleration[1] = lastPose.vecAngularAcceleration[1];
					pose.vecAngularAcceleration[2] = lastPose.vecAngularAcceleration[2];
				} else {
					vr::HmdVector3d_t p;
					p.v[0] = (pose.vecPosition[0] - lastPose.vecPosition[0]) / tdiff;
					if (p.v[0] > -0.01 && p.v[0] < 0.01) { // Set very small values to zero to avoid jitter
						p.v[0] = 0.0;
					}
					p.v[1] = (pose.vecPosition[1] - lastPose.vecPosition[1]) / tdiff;
					if (p.v[1] > -0.01 && p.v[1] < 0.01) {
						p.v[1] = 0.0;
					}
					p.v[2] = (pose.vecPosition[2] - lastPose.vecPosition[2]) / tdiff;
					if (p.v[2] > -0.01 && p.v[2] < 0.01) {
						p.v[2] = 0.0;
					}
					deviceInfo->velMovingAverage().push(p);
					auto vel = deviceInfo->velMovingAverage().average();
					pose.vecVelocity[0] = vel.v[0];
					pose.vecVelocity[1] = vel.v[1];
					pose.vecVelocity[2] = vel.v[2];
					// Predicting acceleration values leads to a very jittery experience.
					// Also, the lighthouse driver does not send acceleration values any way, so why care?
					setAccToZero = true;
					setAngVelToZero = true;
					setAngAccToZero = true;
				}
			} else {
				// Linear approximation is not ready yet, so set everything to zero
				setVelToZero = true;
				setAccToZero = true;
				setAngVelToZero = true;
				setAngAccToZero = true;
			}
		}
		deviceInfo->setLastDriverPose(pose, now);

		// convert back to driver space
		pose.qRotation = pose.qWorldFromDriverRotation * compensatedPoseWorldRot;
		auto adjPoseDriverPos = vrmath::quaternionRotateVector(pose.qWorldFromDriverRotation, tmpConj, compensatedPoseWorldPos + pose.vecWorldFromDriverTranslation);
		pose.vecPosition[0] = adjPoseDriverPos.v[0];
		pose.vecPosition[1] = adjPoseDriverPos.v[1];
		pose.vecPosition[2] = adjPoseDriverPos.v[2];
		if (compensatedPoseWorldVelValid) {
			auto adjPoseDriverVel = vrmath::quaternionRotateVector(pose.qWorldFromDriverRotation, tmpConj, compensatedPoseWorldVel);
			pose.vecVelocity[0] = adjPoseDriverVel.v[0];
			pose.vecVelocity[1] = adjPoseDriverVel.v[1];
			pose.vecVelocity[2] = adjPoseDriverVel.v[2];
		} else if (setVelToZero) {
			pose.vecVelocity[0] = 0.0;
			pose.vecVelocity[1] = 0.0;
			pose.vecVelocity[2] = 0.0;
		}
		if (setAccToZero) {
			pose.vecAcceleration[0] = 0.0;
			pose.vecAcceleration[1] = 0.0;
			pose.vecAcceleration[2] = 0.0;
		}
		if (setAngVelToZero) {
			pose.vecAngularVelocity[0] = 0.0;
			pose.vecAngularVelocity[1] = 0.0;
			pose.vecAngularVelocity[2] = 0.0;
		}
		if (setAngAccToZero) {
			pose.vecAngularAcceleration[0] = 0.0;
			pose.vecAngularAcceleration[1] = 0.0;
			pose.vecAngularAcceleration[2] = 0.0;
		}

		return true;
	} else {
		return true;
	}
}


void MotionCompensationManager::runFrame() {
	if (_motionCompensationEnabled && _motionCompensationStatus == MotionCompensationStatus::WaitingForZeroRef) {
		_motionCompensationZeroRefTimeout++;
		if (_motionCompensationZeroRefTimeout >= _motionCompensationZeroRefTimeoutMax) {
			_motionCompensationRefDevice->setDefaultMode();
			m_parent->sendReplySetMotionCompensationMode(false);
		}
	}
}


}
}
