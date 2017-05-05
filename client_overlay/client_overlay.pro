#-------------------------------------------------
#
# Project created by QtCreator 2015-06-10T16:57:45
#
#-------------------------------------------------

QT       += core gui qml quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenVR-InputEmulatorOverlay
TEMPLATE = app


SOURCES += src/main.cpp\
		src/overlaycontroller.cpp \
		src/tabcontrollers/DeviceManipulationTabController.cpp

HEADERS  += src/overlaycontroller.h \
		src/logging.h \
		src/tabcontrollers/DeviceManipulationTabController.h

INCLUDEPATH += ../openvr/headers \
			../third-party/easylogging++

LIBS += -L../openvr/lib/win64 -lopenvr_api

DESTDIR = bin/win64
