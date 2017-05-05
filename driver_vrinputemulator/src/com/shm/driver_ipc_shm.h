#pragma once

#include <thread>
#include <string>
#include <map>
#include <memory>
#include <boost/interprocess/ipc/message_queue.hpp>


// driver namespace
namespace vrinputemulator {
namespace driver {

// forward declarations
class CServerDriver;


class IpcShmCommunicator {
public:
	void init(CServerDriver* driver);
	void shutdown();

private:
	static void _ipcThreadFunc(IpcShmCommunicator* _this, CServerDriver* driver);

	CServerDriver* _driver = nullptr;
	std::thread _ipcThread;
	volatile bool _ipcThreadRunning = false;
	volatile bool _ipcThreadStopFlag = false;
	std::string _ipcQueueName = "driver_vrinputemulator.server_queue";
	uint32_t _ipcClientIdNext = 1;
	std::map<uint32_t, std::shared_ptr<boost::interprocess::message_queue>> _ipcEndpoints;
};


} // end namespace driver
} // end namespace vrinputemulator
