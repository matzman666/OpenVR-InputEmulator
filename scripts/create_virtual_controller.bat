@echo off

IF [%1] == [] (
	SET SERIAL=controller01
) ELSE (
	SET "SERIAL=%~1"
)

REM client_commandline.exe returns a virtual device id we need for the calls below
for /f %%i in ('client_commandline.exe addcontroller "%SERIAL%"') do set VirtualDeviceId=%%i

IF [%VirtualDeviceId%] == [Error] (
	echo "Couldn't create virtual controller"
) ELSE (

	REM Set device properties
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 1000	string	lighthouse
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 1001	string	"Vive Controller MV"
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 1003	string	vr_controller_vive_1_5
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 1004	bool	0
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 1005	string	HTC
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 1006	string	"1465809478 htcvrsoftware@firmware-win32 2016-06-13 FPGA 1.6/0/0 VRC 1465809477 Radio 1466630404"
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 1007	string	"product 129 rev 1.5.0 lot 2000/0/0 0"
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 1010	bool	1
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 1017	uint64	2164327680
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 1018	uint64	1465809478
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 1029	int32	2
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 3001	uint64	12884901895
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 3002	int32	1
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 3003	int32	3
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 3004	int32	0
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 3005	int32	0
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 3006	int32	0
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 3007	int32	0
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 5000	string	icons
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 5001	string	{htc}controller_status_off.png
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 5002	string	{htc}controller_status_searching.gif
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 5003	string	{htc}controller_status_searching_alert.gif
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 5004	string	{htc}controller_status_ready.png
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 5005	string	{htc}controller_status_ready_alert.png
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 5006	string	{htc}controller_status_error.png
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 5007	string	{htc}controller_status_standby.png
	client_commandline.exe setdeviceproperty %VirtualDeviceId% 5008	string	{htc}controller_status_ready_low.png

	REM Let OpenVR know that there is a new device
	client_commandline.exe publishdevice %VirtualDeviceId%

	REM New devices are initially in disconnected state
	client_commandline.exe setdeviceconnection %VirtualDeviceId% 1
	
	REM set device position
	client_commandline.exe setdeviceposition %VirtualDeviceId% 0 0 0
)
