import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import matzman666.inputemulator 1.0

MyStackViewPage {
    id: deviceOffsetsPage
    headerText: "Device Offsets"

    property int deviceIndex: -1

    function setDeviceIndex(index) {
        deviceIndex = index
        updateDeviceInfo()
        worldFromDriverOffsetBox.updateGUI()
        driverFromHeadOffsetBox.updateGUI()
        driverOffsetBox.updateGUI()
    }

    function updateDeviceInfo() {
        if (deviceIndex >= 0) {
            worldFromDriverOffsetBox.updateValues()
            driverFromHeadOffsetBox.updateValues()
            driverOffsetBox.updateValues()
            deviceOffsetsEnableToggle.checked = DeviceManipulationTabController.deviceOffsetsEnabled(deviceIndex)
        }
    }

    content: ColumnLayout {
        spacing: 18

        MyToggleButton {
            id: deviceOffsetsEnableToggle
            text: "Enable Offsets"
            Layout.fillWidth: false
            onCheckedChanged: {
                DeviceManipulationTabController.enableDeviceOffsets(deviceIndex, checked)
            }
        }

        MyOffsetGroupBox {
            boxTitle: "WorldFromDriver Offsets"
            id: worldFromDriverOffsetBox
            keyboardUIDBase: 200
            setTranslationOffset: function(x, y, z) {
                if (deviceIndex >= 0) {
                    DeviceManipulationTabController.setWorldFromDriverTranslationOffset(deviceIndex, x, y, z)
                }
            }
            setRotationOffset: function(yaw, pitch, roll) {
                if (deviceIndex >= 0) {
                    DeviceManipulationTabController.setWorldFromDriverRotationOffset(deviceIndex, yaw, pitch, roll)
                }
            }
            updateValues: function() {
                var hasChanged = false
                var value = DeviceManipulationTabController.getWorldFromDriverRotationOffset(deviceIndex, 0)
                if (offsetYaw != value) {
                    offsetYaw = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getWorldFromDriverRotationOffset(deviceIndex, 1)
                if (offsetPitch != value) {
                    offsetPitch = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getWorldFromDriverRotationOffset(deviceIndex, 2)
                if (offsetRoll != value) {
                    offsetRoll = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getWorldFromDriverTranslationOffset(deviceIndex, 0)
                if (offsetX != value) {
                    offsetX = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getWorldFromDriverTranslationOffset(deviceIndex, 1)
                if (offsetY != value) {
                    offsetY = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getWorldFromDriverTranslationOffset(deviceIndex, 2)
                if (offsetZ != value) {
                    offsetZ = value
                    hasChanged = true
                }
                if (hasChanged) {
                    updateGUI()
                }
            }
        }

        MyOffsetGroupBox {
            boxTitle: "DriverFromHead Offsets"
            id: driverFromHeadOffsetBox
            keyboardUIDBase: 210
            setTranslationOffset: function(x, y, z) {
                if (deviceIndex >= 0) {
                    DeviceManipulationTabController.setDriverFromHeadTranslationOffset(deviceIndex, x, y, z)
                }
            }
            setRotationOffset: function(yaw, pitch, roll) {
                if (deviceIndex >= 0) {
                    DeviceManipulationTabController.setDriverFromHeadRotationOffset(deviceIndex, yaw, pitch, roll)
                }
            }
            updateValues: function() {
                var hasChanged = false
                var value = DeviceManipulationTabController.getDriverFromHeadRotationOffset(deviceIndex, 0)
                if (offsetYaw != value) {
                    offsetYaw = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getDriverFromHeadRotationOffset(deviceIndex, 1)
                if (offsetPitch != value) {
                    offsetPitch = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getDriverFromHeadRotationOffset(deviceIndex, 2)
                if (offsetRoll != value) {
                    offsetRoll = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getDriverFromHeadTranslationOffset(deviceIndex, 0)
                if (offsetX != value) {
                    offsetX = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getDriverFromHeadTranslationOffset(deviceIndex, 1)
                if (offsetY != value) {
                    offsetY = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getDriverFromHeadTranslationOffset(deviceIndex, 2)
                if (offsetZ != value) {
                    offsetZ = value
                    hasChanged = true
                }
                if (hasChanged) {
                    updateGUI()
                }
            }
        }

        MyOffsetGroupBox {
            boxTitle: "Driver Offsets"
            id: driverOffsetBox
            keyboardUIDBase: 220
            setTranslationOffset: function(x, y, z) {
                if (deviceIndex >= 0) {
                    DeviceManipulationTabController.setDriverTranslationOffset(deviceIndex, x, y, z)
                }
            }
            setRotationOffset: function(yaw, pitch, roll) {
                if (deviceIndex >= 0) {
                    DeviceManipulationTabController.setDriverRotationOffset(deviceIndex, yaw, pitch, roll)
                }
            }
            updateValues: function() {
                var hasChanged = false
                var value = DeviceManipulationTabController.getDriverRotationOffset(deviceIndex, 0)
                if (offsetYaw != value) {
                    offsetYaw = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getDriverRotationOffset(deviceIndex, 1)
                if (offsetPitch != value) {
                    offsetPitch = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getDriverRotationOffset(deviceIndex, 2)
                if (offsetRoll != value) {
                    offsetRoll = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getDriverTranslationOffset(deviceIndex, 0)
                if (offsetX != value) {
                    offsetX = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getDriverTranslationOffset(deviceIndex, 1)
                if (offsetY != value) {
                    offsetY = value
                    hasChanged = true
                }
                value = DeviceManipulationTabController.getDriverTranslationOffset(deviceIndex, 2)
                if (offsetZ != value) {
                    offsetZ = value
                    hasChanged = true
                }
                if (hasChanged) {
                    updateGUI()
                }
            }
        }

        RowLayout {
            MyPushButton {
                Layout.preferredWidth: 200
                text: "Clear"
                onClicked: {
                    worldFromDriverOffsetBox.setTranslationOffset(0,0,0)
                    worldFromDriverOffsetBox.setRotationOffset(0,0,0)
                    driverFromHeadOffsetBox.setTranslationOffset(0,0,0)
                    driverFromHeadOffsetBox.setRotationOffset(0,0,0)
                    driverOffsetBox.setTranslationOffset(0,0,0)
                    driverOffsetBox.setRotationOffset(0,0,0)
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Connections {
            target: DeviceManipulationTabController
            onDeviceInfoChanged: {
                if (index == deviceIndex) {
                    updateDeviceInfo()
                }
            }
        }

    }

}

