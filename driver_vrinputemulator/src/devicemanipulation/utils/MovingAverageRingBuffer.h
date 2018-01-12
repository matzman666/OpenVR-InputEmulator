#pragma once

#include <openvr_driver.h>
#include <openvr_math.h>
#include <mutex>

// driver namespace
namespace vrinputemulator {
namespace driver {


class MovingAverageRingBuffer {
public:
	MovingAverageRingBuffer() noexcept : _buffer(new vr::HmdVector3d_t[1]), _bufferSize(1) {}
	MovingAverageRingBuffer(unsigned size) noexcept : _buffer(new vr::HmdVector3d_t[size > 0 ? size : 1]), _bufferSize(size > 0 ? size : 1) {}
	~MovingAverageRingBuffer() noexcept {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_buffer) {
			delete[] _buffer;
			_buffer = nullptr;
			_bufferSize = _dataStart = _dataSize = 0;
		}
	}

	void resize(unsigned size) noexcept {
		std::lock_guard<std::mutex> lock(_mutex);
		if (size == 0) {
			size = 1;
		}
		if (_buffer) {
			delete[] _buffer;
		}
		_buffer = new vr::HmdVector3d_t[size];
		_bufferSize = size;
		_dataSize = _dataStart = 0;
	}

	unsigned bufferSize() noexcept { return _bufferSize; }

	unsigned dataSize() noexcept { return _dataSize; }

	void push(const vr::HmdVector3d_t& value) {
		if (_dataSize < _bufferSize) {
			_buffer[(_dataStart + _dataSize) % _bufferSize] = value;
			_dataSize++;
		} else {
			_buffer[_dataStart] = value;
			_dataStart = (_dataStart + 1) % _bufferSize;
		}
	}

	vr::HmdVector3d_t average() {
		if (_dataSize > 0) {
			vr::HmdVector3d_t sum = { 0.0, 0.0, 0.0 };
			for (unsigned i = 0; i < _dataSize; i++) {
				sum = sum + _buffer[(_dataStart + i) % _bufferSize];
			}
			return sum / _dataSize;
		} else {
			return vr::HmdVector3d_t();
		}
	}

private:
	std::mutex _mutex;
	vr::HmdVector3d_t* _buffer;
	unsigned _bufferSize;
	unsigned _dataStart = 0;
	unsigned _dataSize = 0;
};

}
}
