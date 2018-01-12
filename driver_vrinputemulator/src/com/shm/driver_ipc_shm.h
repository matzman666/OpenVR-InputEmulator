#pragma once

#include <thread>
#include <string>
#include <map>
#include <mutex>
#include <memory>
#include <boost/interprocess/ipc/message_queue.hpp>


// driver namespace
namespace vrinputemulator {

// forward declarations
namespace ipc { struct Reply; }

namespace driver {

// forward declarations
class ServerDriver;


class IpcShmCommunicator {
public:
	void init(ServerDriver* driver);
	void shutdown();

	void sendReplySetMotionCompensationMode(bool success);

private:
	static void _ipcThreadFunc(IpcShmCommunicator* _this, ServerDriver* driver);

	void sendReply(uint32_t clientId, const ipc::Reply& reply);

	std::mutex _sendMutex;
	ServerDriver* _driver = nullptr;
	std::thread _ipcThread;
	volatile bool _ipcThreadRunning = false;
	volatile bool _ipcThreadStopFlag = false;
	std::string _ipcQueueName = "driver_vrinputemulator.server_queue";
	uint32_t _ipcClientIdNext = 1;
	std::map<uint32_t, std::shared_ptr<boost::interprocess::message_queue>> _ipcEndpoints;

	// This is not exactly multi-user safe, maybe I fix it in the future
	uint32_t _setMotionCompensationClientId = 0;
	uint32_t _setMotionCompensationMessageId = 0;
};


} // end namespace driver
} // end namespace vrinputemulator
