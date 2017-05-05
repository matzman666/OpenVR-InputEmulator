#pragma once


void listDevices(int argc, const char* argv[]);

void buttonEvent(int argc, const char* argv[]);

void axisEvent(int argc, const char* argv[]);

void proximitySensorEvent(int argc, const char* argv[]);

void listVirtual(int argc, const char* argv[]);

void addTrackedController(int argc, const char* argv[]);

void publishTrackedDevice(int argc, const char* argv[]);

void setDeviceProperty(int argc, const char* argv[]);

void getDeviceProperty(int argc, const char* argv[]);

void removeDeviceProperty(int argc, const char* argv[]);

void setDeviceConnection(int argc, const char* argv[]);

void setDevicePosition(int argc, const char* argv[]);

void setDeviceRotation(int argc, const char* argv[]);

void deviceButtonMapping(int argc, const char* argv[]);

void deviceOffsets(int argc, const char* argv[]);

void deviceModes(int argc, const char* argv[]);

void benchmarkIPC(int argc, const char* argv[]);
