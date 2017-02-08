![language](https://img.shields.io/badge/Language-C%2B%2B11-green.svg)  ![dependencies](https://img.shields.io/badge/Dependencies-Boost%201.63-green.svg)  ![license_gpl3](https://img.shields.io/badge/License-GPL%203.0-green.svg)

# OpenVR-InputEmulator

An OpenVR driver that allows to create virtual controllers, emulate controller input, manipulate poses of existing controllers and remap buttons. A client-side library which communicates with the driver via shared-memory is also included.

The OpenVR driver hooks into the HTC Vive lighthouse driver and allows to modify any pose updates or button/axis events coming from the Vive controllers. Due to the nature of this hack the driver may break when Valve decides to update the driver-side OpenVR API.

The motivation of this driver is that I want to make myself a tracked gun that is guaranteed to work in any SteamVR game regardless of whether the original dev wants to support tracked guns or not. To accomplish this I need some way to add translation and rotation offsets to the poses of the motion controllers so that I can line up my tracked gun and the gun in the game. Additionally I need a way to easily switch between the tracking puck on my gun and my motion controller with the game thinking it's still the same controller (Throwing grenades with a tracked gun is not fun). But this driver should also support other use cases. 

There is also a client-side API which other programs can use to communicate with the driver. This API should be powerful enough to also support the development of full-fledged motion-controller drivers.

# Features

- Create virtual controllers and control their positions and rotations.
- Emulate controller input.
- Remap controller buttons.
- Add translation and rotation offsets to the pose of existing controllers.
- Mirror the pose from one controller to another.
- ...

# Notes:

This is a work-in-progress.

# Usage

## Driver

Download the newest driver archive from the [release section](https://github.com/matzman666/OpenVR-InputEmulator/releases) and copy the contained directory into "<Your SteamVR directory>\drivers".

The **activateMultipleDrivers** setting must be set to true, otherwise SteamVR will not load the driver.

## Client

Currently there is only a command line client available. Download the newest client archive from the [release section](https://github.com/matzman666/OpenVR-InputEmulator/releases) and unpack it. Enter client_commandline.exe help on the command line for usage instructions.


# Documentation

## client_commandline commands:

### listdevices

Lists all openvr devices.

### buttonevent

```
buttonevent [press|pressandhold|unpress|touch|touchandhold|untouch] <openvrId> <buttonId>
```

Emulates a button event on the given device. See [openvr.h](https://github.com/ValveSoftware/openvr/blob/master/headers/openvr.h#L600-L626) for available button ids.

### axisevent

```
axisevent <openvrId> <axisId> <x> <y>
```

Emulates an axisevent on the given device. Valid axis ids are 0-4.

### proximitysensor

```
proximitysensor <openvrId> [0|1]
```

Emulates a proximity sensor event on the given device.

### getdeviceproperty

```
getdeviceproperty <openvrId> scan
```

Scans the given device for all available device properties.

```
getdeviceproperty <openvrId> <property> [int32|uint64|float|bool|string]
```

Returns the given device property. See [openvr.h](https://github.com/ValveSoftware/openvr/blob/master/headers/openvr.h#L235-L363) for valid property ids.

### listvirtual

Lists all virtual devices managed by this driver.

### addcontroller

```
addcontroller <serialnumber>
```

Creates a new virtual controller. Serialnumber needs to be unique. When the command is successful the id of the virtual controller is written to stdout.

### publishdevice

```
publishdevice <virtualId>
```

Tells OpenVR that there is a new device. Before this command is called all device properties should have been set.

### setdeviceproperty

```
setdeviceproperty <virtualId> <property> [int32|uint64|float|bool|string] <value>
```

Sets the given device property. See [openvr.h](https://github.com/ValveSoftware/openvr/blob/master/headers/openvr.h#L235-L363) for valid property ids.

### removedeviceproperty

```
removedeviceproperty <virtualId> <property>
```

Removes the given device property. See [openvr.h](https://github.com/ValveSoftware/openvr/blob/master/headers/openvr.h#L235-L363) for valid property ids.

### setdeviceconnection

```
setdeviceconnection <virtualId> [0|1]
```

Sets the connection state of the given virtual device. Default value is disconnected.

### setdeviceposition

```
setdeviceposition <virtualId> <x> <y> <z>
```

Sets the position of the given virtual device.

### setdevicerotation

```
setdeviceposition <virtualId> <x> <y> <z>
```

setdevicerotation <virtualId> <yaw> <pitch> <roll>.

### devicebuttonmapping

```
devicebuttonmapping <openvrId> [enable|disable]
```

Enables/disables device button mapping on the given device.

```
devicebuttonmapping <openvrId> add <buttonId> <mappedButtonId>
```

Adds a new button mapping to the given device. See [openvr.h](https://github.com/ValveSoftware/openvr/blob/master/headers/openvr.h#L600-L626) for available button ids.

```
devicebuttonmapping <openvrId> remove [<buttonId>|all]
```

Removes a button mapping from the given device. 

### devicetranslationoffset

```
devicetranslationoffset <openvrId> [enable|disable]
```

Enables/disables pose translation offset on the given device.

```
devicetranslationoffset <openvrId> set <x> <y> <z>
```

Sets the pose translation offset on the given device.

### devicerotationoffset

```
devicerotationoffset <openvrId> [enable|disable]
```

Enables/disables pose rotation offset on the given device.

```
devicerotationoffset <openvrId> set <yaw> <pitch> <roll>
```

Sets the pose rotation offset on the given device.

### devicemirrormode

```
devicemirrormode <openvrId> off
```

Turns mirror mode off on the given device.

```
devicemirrormode <openvrId> [mirror|redirect] <targetOpenvrId>
```

Mirrors/Redirects the pose of the given device to another device.

## Client API

ToDo. See [vrinputemulator.h](https://github.com/matzman666/OpenVR-InputEmulator/blob/master/lib_vrinputemulator/include/vrinputemulator.h).

# Examples

## Create virtual controller

```
# Create virtual controller
client_commandline.exe addcontroller controller01 # Writes virtual device id to stdout (Let's assume it is 0)
# Set device properties
client_commandline.exe setdeviceproperty 0 1000	string	lighthouse
client_commandline.exe setdeviceproperty 0 1001	string	"Vive Controller MV"
client_commandline.exe setdeviceproperty 0 1003	string	vr_controller_vive_1_5
client_commandline.exe setdeviceproperty 0 1004	bool	0
client_commandline.exe setdeviceproperty 0 1005	string	HTC
client_commandline.exe setdeviceproperty 0 1006	string	"1465809478 htcvrsoftware@firmware-win32 2016-06-13 FPGA 1.6/0/0 VRC 1465809477 Radio 1466630404"
client_commandline.exe setdeviceproperty 0 1007	string	"product 129 rev 1.5.0 lot 2000/0/0 0"
client_commandline.exe setdeviceproperty 0 1010	bool	1
client_commandline.exe setdeviceproperty 0 1017	uint64	2164327680
client_commandline.exe setdeviceproperty 0 1018	uint64	1465809478
client_commandline.exe setdeviceproperty 0 1029	int32	2
client_commandline.exe setdeviceproperty 0 3001	uint64	12884901895
client_commandline.exe setdeviceproperty 0 3002	int32	1
client_commandline.exe setdeviceproperty 0 3003	int32	3
client_commandline.exe setdeviceproperty 0 3004	int32	0
client_commandline.exe setdeviceproperty 0 3005	int32	0
client_commandline.exe setdeviceproperty 0 3006	int32	0
client_commandline.exe setdeviceproperty 0 3007	int32	0
client_commandline.exe setdeviceproperty 0 5000	string	icons
client_commandline.exe setdeviceproperty 0 5001	string	{htc}controller_status_off.png
client_commandline.exe setdeviceproperty 0 5002	string	{htc}controller_status_searching.gif
client_commandline.exe setdeviceproperty 0 5003	string	{htc}controller_status_searching_alert.gif
client_commandline.exe setdeviceproperty 0 5004	string	{htc}controller_status_ready.png
client_commandline.exe setdeviceproperty 0 5005	string	{htc}controller_status_ready_alert.png
client_commandline.exe setdeviceproperty 0 5006	string	{htc}controller_status_error.png
client_commandline.exe setdeviceproperty 0 5007	string	{htc}controller_status_standby.png
client_commandline.exe setdeviceproperty 0 5008	string	{htc}controller_status_ready_low.png
# Let OpenVR know that there is a new device
client_commandline.exe publishdevice 0
# Connect the device
client_commandline.exe setdeviceconnection 0 1
# Set the device position
client_commandline.exe setdeviceposition 0 -1 -1 -1
```

## Map the grip-button to the trigger-button and vice-versa on the controller with id 3

```
client_commandline.exe devicebuttonmapping 3 add 2 33
client_commandline.exe devicebuttonmapping 3 add 33 2
client_commandline.exe devicebuttonmapping 3 enable
```


# Known Bugs

- The shared-memory message queue is prone to deadlock the driver when the client crashes or is exited ungracefully.

# License

This software is released under GPL 3.0.
