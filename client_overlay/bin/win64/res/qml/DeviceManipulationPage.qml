import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import matzman666.inputemulator 1.0
import "." // QTBUG-34418, singletons require explicit import to load qmldir file





MyStackViewPage {
    id: devicePage
    width: 1200
    height: 800
    headerText: "Input Emulator"
    headerShowBackButton: false

    property int deviceIndex: 0

    MyDialogOkPopup {
        id: deviceManipulationMessageDialog
        function showMessage(title, text) {
            dialogTitle = title
            dialogText = text
            open()
        }
    }


    MyDialogOkCancelPopup {
        id: deviceManipulationDeleteProfileDialog
        property int profileIndex: -1
        dialogTitle: "Delete Profile"
        dialogText: "Do you really want to delete this profile?"
        onClosed: {
            if (okClicked) {
                DeviceManipulationTabController.deleteDeviceManipulationProfile(profileIndex)
            }
        }
    }

    MyDialogOkCancelPopup {
        id: deviceManipulationNewProfileDialog
        dialogTitle: "Create New Profile"
        dialogWidth: 600
        dialogHeight: 400
        dialogContentItem: ColumnLayout {
            RowLayout {
                Layout.topMargin: 16
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                MyText {
                    text: "Name: "
                }
                MyTextField {
                    id: deviceManipulationNewProfileName
                    color: "#cccccc"
                    text: ""
                    Layout.fillWidth: true
                    font.pointSize: 20
                    function onInputEvent(input) {
                        text = input
                    }
                }
            }
            ColumnLayout {
                Layout.topMargin: 16
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                MyToggleButton {
                    id: includeDeviceOffsetsToggle
                    text: "Include Device Offsets"
                }
                MyToggleButton {
                    id: includeInputRemapping
                    text: "Include Input Remapping"
                }
            }
        }
        onClosed: {
            if (okClicked) {
                if (deviceManipulationNewProfileName.text == "") {
                    deviceManipulationMessageDialog.showMessage("Create New Profile", "ERROR: Empty profile name.")
                } else if (!includeDeviceOffsetsToggle.checked && !includeInputRemapping.checked) {
                    deviceManipulationMessageDialog.showMessage("Create New Profile", "ERROR: Nothing to include selected.")
                } else {
                    DeviceManipulationTabController.addDeviceManipulationProfile(deviceManipulationNewProfileName.text, deviceIndex,
                        includeDeviceOffsetsToggle.checked, includeInputRemapping.checked)
                }

            }
        }
        function openPopup(device) {
            deviceManipulationNewProfileName.text = ""
            includeDeviceOffsetsToggle.checked = false
            includeInputRemapping.checked = false
            deviceIndex = device
            open()
        }
    }



    content: ColumnLayout {
        spacing: 18

        RowLayout {
            spacing: 18
            Layout.bottomMargin: 64

            MyText {
                text: "Device:"
            }

            MyComboBox {
                id: deviceSelectionComboBox
                Layout.maximumWidth: 799
                Layout.minimumWidth: 799
                Layout.preferredWidth: 799
                Layout.fillWidth: true
                model: []
                onCurrentIndexChanged: {
                    deviceModeSelectionComboBox.currentIndex = 0
					if (currentIndex >= 0) {
						DeviceManipulationTabController.updateDeviceInfo(currentIndex);
						fetchDeviceInfo()
                    }
                }
            }

            MyPushButton {
                id: deviceIdentifyButton
                enabled: deviceSelectionComboBox.currentIndex >= 0
                Layout.preferredWidth: 194
                text: "Identify"
                onClicked: {
                    if (deviceSelectionComboBox.currentIndex >= 0) {
                        DeviceManipulationTabController.triggerHapticPulse(deviceSelectionComboBox.currentIndex)
                    }
                }
            }
        }

        RowLayout {
            spacing: 18

            MyText {
                text: "Status:"
            }

            MyText {
                id: deviceStatusText
                text: ""
            }
        }

        RowLayout {
            spacing: 18
            MyText {
                text: "Device Mode:"
            }

            MyComboBox {
                id: deviceModeSelectionComboBox
                enabled: false
                Layout.maximumWidth: 350
                Layout.minimumWidth: 350
                Layout.preferredWidth: 400
                Layout.fillWidth: true
                model: ["Default", "Disable", "Redirect to", "Swap with", "Motion Compensation"]
                onCurrentIndexChanged: {
                    if (currentIndex == 2 || currentIndex == 3) {
                        deviceModeTargetSelectionComboBox.enabled = true
                        var targetDevices = []
                        var deviceCount = DeviceManipulationTabController.getDeviceCount()
                        for (var i = 0; i < deviceCount; i++) {
                            var deviceMode = DeviceManipulationTabController.getDeviceMode(i)
                            var deviceId = DeviceManipulationTabController.getDeviceId(i)
                            if (i != deviceSelectionComboBox.currentIndex) {
                                var deviceName =deviceId + ": " + DeviceManipulationTabController.getDeviceSerial(i)
                                var deviceClass = DeviceManipulationTabController.getDeviceClass(i)
                                if (deviceClass == 1) {
                                    deviceName += " (HMD)"
                                } else if (deviceClass == 2) {
                                    deviceName += " (Controller)"
                                } else if (deviceClass == 3) {
                                    deviceName += " (Tracker)"
                                } else if (deviceClass == 4) {
                                    deviceName += " (Base-Station)"
                                } else {
                                    deviceName += " (Unknown " + deviceClass.toString() + ")"
                                }
                                deviceModeTargetSelectionComboBox.deviceIndices.push(i)
                                targetDevices.push(deviceName)
                            }
                        }
                        deviceModeTargetSelectionComboBox.model = targetDevices
                    } else {
                        deviceModeTargetSelectionComboBox.enabled = false
                        deviceModeTargetSelectionComboBox.model = []
                        deviceModeTargetSelectionComboBox.deviceIndices = []
                    }
                }
            }

            MyComboBox {
                id: deviceModeTargetSelectionComboBox
                property var deviceIndices: []
                Layout.maximumWidth: 350
                Layout.minimumWidth: 350
                Layout.preferredWidth: 300
                Layout.fillWidth: true
                enabled: false
                model: []
                onCurrentIndexChanged: {
                }
            }

            MyPushButton {
                id: deviceModeApplyButton
                Layout.preferredWidth: 200
                enabled: false
                text: "Apply"
                onClicked: {
                    if (deviceModeSelectionComboBox.currentIndex == 2 || deviceModeSelectionComboBox.currentIndex == 3) {
                        var targetId = deviceModeTargetSelectionComboBox.deviceIndices[deviceModeTargetSelectionComboBox.currentIndex]
                        if (targetId >= 0) {
                            if (!DeviceManipulationTabController.setDeviceMode(deviceSelectionComboBox.currentIndex, deviceModeSelectionComboBox.currentIndex, targetId)) {
                                deviceManipulationMessageDialog.showMessage("Set Device Mode", "Could not set device mode: " + DeviceManipulationTabController.getDeviceModeErrorString())
                            }
                        }
                    } else {
                        if (!DeviceManipulationTabController.setDeviceMode(deviceSelectionComboBox.currentIndex, deviceModeSelectionComboBox.currentIndex, 0)) {
                            deviceManipulationMessageDialog.showMessage("Set Device Mode", "Could not set device mode: " + DeviceManipulationTabController.getDeviceModeErrorString())
                        }
                    }
                }
            }
        }


        RowLayout {
            spacing: 18

            MyPushButton {
                id: deviceManipulationOffsetButton
                enabled: false
                activationSoundEnabled: false
                Layout.preferredWidth: 548
                text: "Device Offsets"
                onClicked: {
                    if (deviceSelectionComboBox.currentIndex >= 0) {
                        deviceOffsetsPage.setDeviceIndex(deviceSelectionComboBox.currentIndex)
                        MyResources.playFocusChangedSound()
                        var res = mainView.push(deviceOffsetsPage)
                    }
                }
            }

            MyPushButton {
                id: motionCompensationButton
                activationSoundEnabled: false
                Layout.preferredWidth: 548
                enabled: false
                text: "Motion Compensation Settings"
                onClicked: {
                    MyResources.playFocusChangedSound()
                    var res = mainView.push(motionCompensationPage)
                }
            }

        }


        RowLayout {
            spacing: 18

            MyPushButton {
                id: deviceRenderModelButton
                enabled: false
                activationSoundEnabled: false
                Layout.preferredWidth: 548
                text: "Render Model"
                onClicked: {
                    if (deviceSelectionComboBox.currentIndex >= 0) {
                        deviceRenderModelPage.setDeviceIndex(deviceSelectionComboBox.currentIndex)
                        MyResources.playFocusChangedSound()
                        var res = mainView.push(deviceRenderModelPage)
                    }
                }
            }

            MyPushButton {
                id: deviceInputRemappingButton
                activationSoundEnabled: false
                Layout.preferredWidth: 548
                enabled: false
                text: "Input Remapping"
                onClicked: {
                    if (deviceSelectionComboBox.currentIndex >= 0) {
                        deviceInputRemappingPage.setDeviceIndex(deviceSelectionComboBox.currentIndex)
                        MyResources.playFocusChangedSound()
                        var res = mainView.push(deviceInputRemappingPage)
                    }
                }
            }

        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        ColumnLayout {
            Layout.bottomMargin: 6
            spacing: 18
            RowLayout {
                spacing: 18

                MyText {
                    text: "Profile:"
                }

                MyComboBox {
                    id: deviceManipulationProfileComboBox
                    Layout.maximumWidth: 799
                    Layout.minimumWidth: 799
                    Layout.preferredWidth: 799
                    Layout.fillWidth: true
                    model: [""]
                    onCurrentIndexChanged: {
                        if (currentIndex > 0) {
                            deviceManipulationApplyProfileButton.enabled = true
                            deviceManipulationDeleteProfileButton.enabled = true
                        } else {
                            deviceManipulationApplyProfileButton.enabled = false
                            deviceManipulationDeleteProfileButton.enabled = false
                        }
                    }
                }

                MyPushButton {
                    id: deviceManipulationApplyProfileButton
                    enabled: false
                    Layout.preferredWidth: 200
                    text: "Apply"
                    onClicked: {
                        if (deviceManipulationProfileComboBox.currentIndex > 0 && deviceSelectionComboBox.currentIndex >= 0) {
                            DeviceManipulationTabController.applyDeviceManipulationProfile(deviceManipulationProfileComboBox.currentIndex - 1, deviceSelectionComboBox.currentIndex);
                            deviceManipulationProfileComboBox.currentIndex = 0
                        }
                    }
                }
            }
            RowLayout {
                spacing: 18
                Item {
                    Layout.fillWidth: true
                }
                MyPushButton {
                    id: deviceManipulationDeleteProfileButton
                    enabled: false
                    Layout.preferredWidth: 200
                    text: "Delete Profile"
                    onClicked: {
                        if (deviceManipulationProfileComboBox.currentIndex > 0) {
                            deviceManipulationDeleteProfileDialog.profileIndex = deviceManipulationProfileComboBox.currentIndex - 1
                            deviceManipulationDeleteProfileDialog.open()
                        }
                    }
                }
                MyPushButton {
                    id: deviceManipulationNewProfileButton
                    Layout.preferredWidth: 200
                    text: "New Profile"
                    onClicked: {
                        if (deviceSelectionComboBox.currentIndex >= 0) {
                            deviceManipulationNewProfileDialog.openPopup(deviceSelectionComboBox.currentIndex)
                        }
                    }
                }
            }
        }

        RowLayout {
            spacing: 18
            Item {
                Layout.fillWidth: true
            }
            MyText {
                id: appVersionText
                text: "v0.0"
            }
        }


        Component.onCompleted: {
            appVersionText.text = OverlayController.getVersionString()
            reloadDeviceManipulationProfiles()
        }

        Connections {
            target: DeviceManipulationTabController
            onDeviceCountChanged: {
                fetchDevices()
                fetchDeviceInfo()
            }
            onDeviceInfoChanged: {
                if (index == deviceSelectionComboBox.currentIndex) {
                    fetchDeviceInfo()
                }
            }
            onDeviceManipulationProfilesChanged: {
                reloadDeviceManipulationProfiles()
            }
        }

    }

    function fetchDevices() {
        var devices = []
        var oldIndex = deviceSelectionComboBox.currentIndex
        var deviceCount = DeviceManipulationTabController.getDeviceCount()
        for (var i = 0; i < deviceCount; i++) {
            var deviceId = DeviceManipulationTabController.getDeviceId(i)
            var deviceName = deviceId.toString() + ": "
            deviceName += DeviceManipulationTabController.getDeviceSerial(i)
            var deviceClass = DeviceManipulationTabController.getDeviceClass(i)
            if (deviceClass == 1) {
                deviceName += " (HMD)"
            } else if (deviceClass == 2) {
                deviceName += " (Controller)"
            } else if (deviceClass == 3) {
                deviceName += " (Tracker)"
            } else if (deviceClass == 4) {
                deviceName += " (Base-Station)"
            } else {
                deviceName += " (Unknown " + deviceClass.toString() + ")"
            }
            devices.push(deviceName)
        }
        deviceSelectionComboBox.model = devices
        if (deviceCount <= 0) {
            deviceSelectionComboBox.currentIndex = -1
            deviceModeSelectionComboBox.enabled = false
            deviceModeApplyButton.enabled = false
            motionCompensationButton.enabled = false
            deviceManipulationOffsetButton.enabled = false
            deviceInputRemappingButton.enabled = false
            deviceRenderModelButton.enabled = false
            deviceIdentifyButton.enabled = false
            deviceManipulationNewProfileButton.enabled = false
            deviceManipulationProfileComboBox.enabled = false
        } else {
            deviceModeSelectionComboBox.enabled = true
            deviceManipulationOffsetButton.enabled = true
            deviceRenderModelButton.enabled = true
            deviceInputRemappingButton.enabled = true
            motionCompensationButton.enabled = true
            deviceModeApplyButton.enabled = true
            deviceIdentifyButton.enabled = true
            deviceManipulationNewProfileButton.enabled = true
            deviceManipulationProfileComboBox.enabled = true
            if (oldIndex >= 0 && oldIndex < deviceCount) {
                deviceSelectionComboBox.currentIndex = oldIndex
            } else {
                deviceSelectionComboBox.currentIndex = 0
            }
        }
    }

    function fetchDeviceInfo() {
        var index = deviceSelectionComboBox.currentIndex
        if (index >= 0) {
            var deviceMode = DeviceManipulationTabController.getDeviceMode(index)
            var deviceState = DeviceManipulationTabController.getDeviceState(index)
            var deviceClass = DeviceManipulationTabController.getDeviceClass(index)
            var statusText = ""
            if (deviceMode == 0) { // default
                deviceModeSelectionComboBox.currentIndex = 0
                deviceModeSelectionComboBox.enabled = true
                if (deviceState == 0) {
                    statusText = "Default"
                } else if (deviceState == 1) {
                    statusText = "Default (Disconnected)"
                } else {
                    statusText = "Default (Unknown state " + deviceState.toString() + ")"
                }
            } else if (deviceMode == 1) { // fake disconnection
                deviceModeSelectionComboBox.currentIndex = 1
                if (deviceState == 0 || deviceState == 1) {
                    statusText = "Disabled"
                } else {
                    statusText = "Disabled (Unknown state " + deviceState.toString() + ")"
                }
            } else if (deviceMode == 2) { // redirect source
                deviceModeSelectionComboBox.currentIndex = 2
                var refIndex = DeviceManipulationTabController.getDeviceModeRefDeviceIndex(index)
                for (var i = 0; i < deviceModeTargetSelectionComboBox.deviceIndices.length; i++) {
                    if (refIndex == deviceModeTargetSelectionComboBox.deviceIndices[i]) {
                        deviceModeTargetSelectionComboBox.currentIndex = i
                        break
                    }
                }
                if (deviceState == 0) {
                    statusText = "Redirect Source"
                } else if (deviceState == 1) {
                    statusText = "Redirect Source (Suspended)"
                } else {
                    statusText = "Redirect Source (Unknown state " + deviceState.toString() + ")"
                }
            } else if (deviceMode == 3) { // redirect target
                deviceModeSelectionComboBox.currentIndex = 2
                var refIndex = DeviceManipulationTabController.getDeviceModeRefDeviceIndex(index)
                for (var i = 0; i < deviceModeTargetSelectionComboBox.deviceIndices.length; i++) {
                    if (refIndex == deviceModeTargetSelectionComboBox.deviceIndices[i]) {
                        deviceModeTargetSelectionComboBox.currentIndex = i
                        break
                    }
                }
                if (deviceState == 0) {
                    statusText = "Redirect Target"
                } else if (deviceState == 1) {
                    statusText = "Redirect Target (Suspended)"
                } else {
                    statusText = "Redirect Target (Unknown state " + deviceState.toString() + ")"
                }
            } else if (deviceMode == 4) { // swap mode
                deviceModeSelectionComboBox.currentIndex = 3
                var refIndex = DeviceManipulationTabController.getDeviceModeRefDeviceIndex(index)
                for (var i = 0; i < deviceModeTargetSelectionComboBox.deviceIndices.length; i++) {
                    if (refIndex == deviceModeTargetSelectionComboBox.deviceIndices[i]) {
                        deviceModeTargetSelectionComboBox.currentIndex = i
                        break
                    }
                }
                if (deviceState == 0) {
                    statusText = "Swapped"
                } else if (deviceState == 1) {
                    statusText = "Swapped (Disconnected)"
                } else {
                    statusText = "Swapped (Unknown state " + deviceState.toString() + ")"
                }
            } else if (deviceMode == 5) { // motion compens4tion
                deviceModeSelectionComboBox.currentIndex = 4
                if (deviceState == 0) {
                    statusText = "Motion Compensation"
                } else {
                    statusText = "Motion Compensation (Unknown state " + deviceState.toString() + ")"
                }
            } else {
                statusText = "Unknown Mode " + deviceMode.toString()
            }
            deviceStatusText.text = statusText
            if (deviceClass == 2 || deviceClass == 3) {
                deviceIdentifyButton.enabled = true
            } else {
                deviceIdentifyButton.enabled = false
            }
        }
    }

    function reloadDeviceManipulationProfiles() {
        var profiles = [""]
        var profileCount = DeviceManipulationTabController.getDeviceManipulationProfileCount()
        for (var i = 0; i < profileCount; i++) {
            profiles.push(DeviceManipulationTabController.getDeviceManipulationProfileName(i))
        }
        deviceManipulationProfileComboBox.model = profiles
        deviceManipulationProfileComboBox.currentIndex = 0
    }

}
