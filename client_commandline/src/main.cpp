#include <iostream>
#include <openvr.h>
#include "client_commandline.h"

#include <vrinputemulator.h>


void printHelp(int argc, const char* argv[]) {
	std::cout << "Usage: client_commandline.exe <command> ..." << std::endl << std::endl
		<< "Available commands (enter \"<command> help\" for help):" << std::endl << std::endl
		<< "  listdevices\t\t\tLists all openvr devices" << std::endl
		<< "  buttonevent\t\t\tSends button event" << std::endl
		<< "  axisevent\t\t\tSends axis event" << std::endl
		<< "  proximitysensor\t\tSends proximity sensor event" << std::endl
		<< "  getdeviceproperty\t\tReturns a device property" << std::endl
		<< "  listvirtual\t\t\tLists all virtual devices" << std::endl
		<< "  addcontroller\t\t\tCreates a new virtual controller" << std::endl
		<< "  publishdevice\t\t\tAdds a virtual controller to openvr" << std::endl
		<< "  setdeviceproperty\t\tSets a device property" << std::endl
		<< "  removedeviceproperty\t\tRemoves a device property" << std::endl
		<< "  setdeviceconnection\t\tSets the connection state of a virtual device" << std::endl
		<< "  setdeviceposition\t\tSets the position of a virtual device" << std::endl
		<< "  setdevicerotation\t\tSets the rotation of a virtual device" << std::endl
		<< "  devicebuttonmapping\t\tConfigures the device button mapping" << std::endl
		<< "  devicetranslationoffset\tConfigure the device translation offset" << std::endl
		<< "  devicerotationoffset\t\tConfigure the device rotation offset" << std::endl
		<< "  devicemirrormode\t\tConfigure the device mirror mode" << std::endl
		<< "  benchmarkipc\t\t\tipc benchmarks" << std::endl;
}


int main(int argc, const char* argv[]) {
	int retval = 0;
	if (argc <= 1) {
		std::cout << "Error: No Arguments given." << std::endl;
		printHelp(argc, argv);
		exit(1);
	}

	try {
		if (argc == 1 || std::strcmp(argv[1], "help") == 0) {
			printHelp(argc, argv);
		} else if (std::strcmp(argv[1], "listdevices") == 0) {
			listDevices(argc, argv);
		} else if (std::strcmp(argv[1], "buttonevent") == 0) {
			buttonEvent(argc, argv);
		} else if (std::strcmp(argv[1], "axisevent") == 0) {
			axisEvent(argc, argv);
		} else if (std::strcmp(argv[1], "proximitysensor") == 0) {
			proximitySensorEvent(argc, argv);
		} else if (std::strcmp(argv[1], "listvirtual") == 0) {
			listVirtual(argc, argv);
		} else if (std::strcmp(argv[1], "addcontroller") == 0) {
			addTrackedController(argc, argv);
		} else if (std::strcmp(argv[1], "publishdevice") == 0) {
			publishTrackedDevice(argc, argv);
		} else if (std::strcmp(argv[1], "setdeviceproperty") == 0) {
			setDeviceProperty(argc, argv);
		} else if (std::strcmp(argv[1], "getdeviceproperty") == 0) {
			getDeviceProperty(argc, argv);
		} else if (std::strcmp(argv[1], "removedeviceproperty") == 0) {
			removeDeviceProperty(argc, argv);
		} else if (std::strcmp(argv[1], "setdeviceconnection") == 0) {
			setDeviceConnection(argc, argv);
		} else if (std::strcmp(argv[1], "setdeviceposition") == 0) {
			setDevicePosition(argc, argv);
		} else if (std::strcmp(argv[1], "setdevicerotation") == 0) {
			setDeviceRotation(argc, argv);
		} else if (std::strcmp(argv[1], "devicebuttonmapping") == 0) {
			deviceButtonMapping(argc, argv);
		} else if (std::strcmp(argv[1], "deviceoffsets") == 0) {
			deviceOffsets(argc, argv);
		} else if (std::strcmp(argv[1], "devicemodes") == 0) {
			deviceModes(argc, argv);
		} else if (std::strcmp(argv[1], "benchmarkipc") == 0) {
			benchmarkIPC(argc, argv);
		} else {
			throw std::runtime_error("Error: Unknown command.");
		}
	} catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		retval = 3;
	}

	return retval;
}

