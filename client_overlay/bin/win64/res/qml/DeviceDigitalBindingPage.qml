import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import matzman666.inputemulator 1.0

MyStackViewPage {
    id: deviceDigitalBindingPage

    headerText: "Button Binding"

    property int deviceIndex: -1

    property int buttonId: -1

    property var _controllerIds: []

    function setDeviceIndex(index, button) {
        deviceIndex = index
        buttonId = button
        var _controllerNames = ["<No Change>"]
        _controllerIds = []
        for (var i = 0; i < DeviceManipulationTabController.getDeviceCount(); i++) {
            var deviceId = DeviceManipulationTabController.getDeviceId(i)
            var deviceName = deviceId.toString() + ": " + DeviceManipulationTabController.getDeviceSerial(i)
            _controllerNames.push(deviceName)
            _controllerIds.push(deviceId)
        }
        openVRControllerComboBox.model = _controllerNames
        var _buttonNames = []
        for (var i = 0; i < DigitalInputRemappingController.getButtonMaxCount(); i++) {
            _buttonNames.push(DigitalInputRemappingController.getButtonName(i, true))
        }
        openvrButtonComboBox.model = _buttonNames
        var keys = []
        for (var i = 0; i < OverlayController.keyboardVirtualCodeCount(); i++) {
            keys.push(OverlayController.keyboardVirtualCodeNameFromIndex(i))
        }
        keyboardKeyComboBox.model = keys
        bindingTypeComboBox.currentIndex = DigitalInputRemappingController.getBindingType()
        if (bindingTypeComboBox.currentIndex == 2) {
            var controllerId = DigitalInputRemappingController.getBindingOpenVRControllerId()
            var index = 0
            for (var i = 0; i < _controllerIds.length; i++) {
                if (controllerId == _controllerIds[i]) {
                    index = i + 1
                    break
                }

            }
            openVRControllerComboBox.currentIndex = index
            openvrButtonComboBox.currentIndex = DigitalInputRemappingController.getBindingOpenVRButtonId()
            toggleModeToggle.checked = DigitalInputRemappingController.isToggleModeEnabled()
            toggleThresholdSlider.value = DigitalInputRemappingController.toggleModeThreshold()
            autoTriggerToggle.checked = DigitalInputRemappingController.isAutoTriggerEnabled()
            triggerFrequencySlider.value = (DigitalInputRemappingController.autoTriggerFrequency() / 100).toFixed(1)
        } else if (bindingTypeComboBox.currentIndex == 3) {
            keyboardShiftToggle.checked = DigitalInputRemappingController.keyboardShiftEnabled()
            keyboardCtrlToggle.checked = DigitalInputRemappingController.keyboardCtrlEnabled()
            keyboardAltToggle.checked = DigitalInputRemappingController.keyboardAltEnabled()
            keyboardUseScanCodeToggle.currentIndex = DigitalInputRemappingController.keyboardUseScanCode() ? 1 : 0
            keyboardKeyComboBox.currentIndex = DigitalInputRemappingController.keyboardKeyIndex()
            toggleModeToggle.checked = DigitalInputRemappingController.isToggleModeEnabled()
            toggleThresholdSlider.value = DigitalInputRemappingController.toggleModeThreshold()
            autoTriggerToggle.checked = DigitalInputRemappingController.isAutoTriggerEnabled()
            triggerFrequencySlider.value = (DigitalInputRemappingController.autoTriggerFrequency() / 100).toFixed(1)
        } else {
            openVRControllerComboBox.currentIndex = 0
            openvrButtonComboBox.currentIndex = 0
            keyboardKeyComboBox.currentIndex = 0
            keyboardShiftToggle.checked = false
            keyboardCtrlToggle.checked = false
            keyboardAltToggle.checked = false
            toggleModeToggle.checked = false
            keyboardUseScanCodeToggle.currentIndex = 1
            toggleThresholdSlider.value = 300
            autoTriggerToggle.checked = false
            triggerFrequencySlider.value = 10
        }
    }

    content: ColumnLayout {

        RowLayout {
            MyText {
                Layout.preferredWidth: 200
                text: "Binding Type:"
            }
            MyComboBox {
                id: bindingTypeComboBox
                Layout.maximumWidth: 910
                Layout.minimumWidth: 910
                Layout.preferredWidth: 910
                Layout.fillWidth: true
                model: [
                    "No Remapping",
                    "Disabled",
                    "OpenVR",
                    "Keyboard",
                    "Suspend Redirect Mode",
                    "Toggle Touchpad Emulation"
                ]
                onCurrentIndexChanged: {
                    if (currentIndex == 2) {
                        openvrConfigContainer.visible = true
                        keyboardConfigContainer.visible = false
                        toggleModeContainer.visible = true
                        autoTriggerContainer.visible = true
                    } else if (currentIndex ==3) {
                        openvrConfigContainer.visible = false
                        keyboardConfigContainer.visible = true
                        toggleModeContainer.visible = true
                        autoTriggerContainer.visible = true
                    } else {
                        openvrConfigContainer.visible = false
                        keyboardConfigContainer.visible = false
                        toggleModeContainer.visible = false
                        autoTriggerContainer.visible = false
                    }
                }
            }
        }

        ColumnLayout {
            id: openvrConfigContainer
            Layout.topMargin: 48
            visible: false
            spacing: 8

            RowLayout {
                MyText {
                    Layout.preferredWidth: 200
                    text: "Controller:"
                }
                MyComboBox {
                    id: openVRControllerComboBox
                    Layout.maximumWidth: 910
                    Layout.minimumWidth: 910
                    Layout.preferredWidth: 910
                    Layout.fillWidth: true
                    model: [
                        "<Original>"
                    ]
                    onCurrentIndexChanged: {
                    }
                }
            }

            RowLayout {
                MyText {
                    Layout.preferredWidth: 200
                    text: "Button:"
                }
                MyComboBox {
                    id: openvrButtonComboBox
                    Layout.maximumWidth: 910
                    Layout.minimumWidth: 910
                    Layout.preferredWidth: 910
                    Layout.fillWidth: true
                    model: []
                    onCurrentIndexChanged: {
                    }
                }
            }
        }

        ColumnLayout {
            id: keyboardConfigContainer
            Layout.topMargin: 48
            visible: false
            spacing: 8
            RowLayout {
                MyText {
                    Layout.preferredWidth: 200
                    text: "Key:"
                }
                MyComboBox {
                    id: keyboardKeyComboBox
                    Layout.maximumWidth: 910
                    Layout.minimumWidth: 910
                    Layout.preferredWidth: 910
                    Layout.fillWidth: true
                    model: []
                    onCurrentIndexChanged: {
                    }
                }
            }
            RowLayout {
                Item {
                    Layout.preferredWidth: 200
                }
                MyToggleButton {
                    id: keyboardShiftToggle
                    Layout.preferredWidth: 174
                    text: "Shift"
                }
                MyToggleButton {
                    id: keyboardCtrlToggle
                    Layout.preferredWidth: 174
                    text: "CTRL"
                }
                MyToggleButton {
                    id: keyboardAltToggle
                    Layout.preferredWidth: 174
                    text: "Alt"
                }
                MyText {
                    text: "Using: "
                    Layout.preferredWidth: 90
                }
                MyComboBox {
                    id: keyboardUseScanCodeToggle
                    Layout.preferredWidth:275
                    model: [
                        "Virtual Key Code",
                        "Scan Code"
                    ]
                }
            }
        }

        ColumnLayout {
            id: toggleModeContainer
            Layout.topMargin: 48
            visible: false
            spacing: 8

            MyToggleButton {
                id: toggleModeToggle
                text: "Enable Toggle Mode"
                onCheckedChanged: {
                    toggleThresholdLabel.enabled = checked
                    toggleThresholdMinusButton.enabled = checked
                    toggleThresholdPlusButton.enabled = checked
                    toggleThresholdSlider.enabled = checked
                    toggleThresholdText.enabled = checked
                }
            }

            RowLayout {
                spacing: 16

                MyText {
                    id: toggleThresholdLabel
                    text: "Toggle Threshold:"
                    Layout.preferredWidth: 350
                    Layout.rightMargin: 12
                    enabled: false
                }

                MyPushButton2 {
                    id: toggleThresholdMinusButton
                    text: "-"
                    Layout.preferredWidth: 40
                    enabled: false
                    onClicked: {
                        toggleThresholdSlider.value -= 100
                    }
                }

                MySlider {
                    id: toggleThresholdSlider
                    from: 0
                    to: 5000
                    stepSize: 100
                    value: 300
                    snapMode: Slider.SnapAlways
                    Layout.fillWidth: true
                    enabled: false
                    onPositionChanged: {
                        var val = this.from + ( this.position  * (this.to - this.from))
                        toggleThresholdText.text = val.toFixed(0) + " ms"
                    }
                    onValueChanged: {
                        toggleThresholdText.text = value.toFixed(0) + " ms"
                    }
                }

                MyPushButton2 {
                    id: toggleThresholdPlusButton
                    text: "+"
                    Layout.preferredWidth: 40
                    enabled: false
                    onClicked: {
                        toggleThresholdSlider.value += 100
                    }
                }

                MyTextField {
                    id: toggleThresholdText
                    text: "0.0"
                    Layout.preferredWidth: 150
                    Layout.leftMargin: 10
                    horizontalAlignment: Text.AlignHCenter
                    enabled: false
                    function onInputEvent(input) {
                        var val = parseFloat(input)
                        if (!isNaN(val)) {
                            if (val < 0) {
                                val = 0
                            }
                            var v = val.toFixed(0)
                            toggleThresholdSlider.value = v
                        } else {
                            toggleThresholdText.text = toggleThresholdSlider.value.toFixed(0) + " ms"
                        }
                    }
                }
            }
        }

        ColumnLayout {
            id: autoTriggerContainer
            Layout.topMargin: 48
            visible: false
            spacing: 8

            MyToggleButton {
                id: autoTriggerToggle
                text: "Enable Auto-Trigger"
                onCheckedChanged: {
                    triggerFrequencyLabel.enabled = checked
                    triggerFrequencyMinusButton.enabled = checked
                    triggerFrequencyPlusButton.enabled = checked
                    triggerFrequencySlider.enabled = checked
                    triggerFrequencyText.enabled = checked
                }
            }

            RowLayout {
                spacing: 16

                MyText {
                    id: triggerFrequencyLabel
                    text: "Trigger Frequency:"
                    Layout.preferredWidth: 350
                    Layout.rightMargin: 12
                    enabled: false
                }

                MyPushButton2 {
                    id: triggerFrequencyMinusButton
                    text: "-"
                    Layout.preferredWidth: 40
                    enabled: false
                    onClicked: {
                        triggerFrequencySlider.value -= 0.1
                    }
                }

                MySlider {
                    id: triggerFrequencySlider
                    from: 0.1
                    to: 45.0
                    stepSize: 0.1
                    value: 1.0
                    snapMode: Slider.SnapAlways
                    Layout.fillWidth: true
                    onPositionChanged: {
                        var val = this.from + ( this.position  * (this.to - this.from))
                        triggerFrequencyText.text = val.toFixed(1) + " Hz"
                    }
                    onValueChanged: {
                        triggerFrequencyText.text = value.toFixed(1) + " Hz"
                    }
                }

                MyPushButton2 {
                    id: triggerFrequencyPlusButton
                    text: "+"
                    Layout.preferredWidth: 40
                    enabled: false
                    onClicked: {
                        triggerFrequencySlider.value += 0.1
                    }
                }

                MyTextField {
                    id: triggerFrequencyText
                    text: "0.0"
                    Layout.preferredWidth: 150
                    Layout.leftMargin: 10
                    horizontalAlignment: Text.AlignHCenter
                    function onInputEvent(input) {
                        var val = parseFloat(input)
                        if (!isNaN(val)) {
                            if (val < 0) {
                                val = 0
                            }
                            var v = val.toFixed(0)
                            triggerFrequencySlider.value = v
                        } else {
                            triggerFrequencyText.text = triggerFrequencySlider.value.toFixed(1) + " Hz"
                        }
                    }
                }
            }
        }


        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        RowLayout {
            Item {
                Layout.fillWidth: true
            }
            MyPushButton {
                Layout.preferredWidth: 200
                text: "Save"
                onClicked: {
                    var toggleMode = toggleModeToggle.checked
                    var toggleDelay = toggleThresholdSlider.value
                    var autoTrigger = autoTriggerToggle.checked
                    var autoTriggerFreq = Math.round(triggerFrequencySlider.value * 100)
                    if (bindingTypeComboBox.currentIndex == 0) {
                        DigitalInputRemappingController.finishConfigureBinding_Original()
                    } else if (bindingTypeComboBox.currentIndex == 1) {
                        DigitalInputRemappingController.finishConfigureBinding_Disabled()
                    } else if (bindingTypeComboBox.currentIndex == 2) {
                        var controllerId = -1
                        if (openVRControllerComboBox.currentIndex > 0) {
                            var controllerId = _controllerIds[openVRControllerComboBox.currentIndex - 1]
                        }
                        var buttonId = openvrButtonComboBox.currentIndex
                        DigitalInputRemappingController.finishConfigureBinding_OpenVR(controllerId, buttonId, toggleMode, toggleDelay, autoTrigger, autoTriggerFreq)
                    } else if (bindingTypeComboBox.currentIndex == 3) {
                        var shift = keyboardShiftToggle.checked
                        var ctrl = keyboardCtrlToggle.checked
                        var alt = keyboardAltToggle.checked
                        var keyIndex = keyboardKeyComboBox.currentIndex
                        var useScanCode = keyboardUseScanCodeToggle.currentIndex == 1
                        DigitalInputRemappingController.finishConfigureBinding_keyboard(shift, ctrl, alt, keyIndex, useScanCode, toggleMode, toggleDelay, autoTrigger, autoTriggerFreq)
                    } else if (bindingTypeComboBox.currentIndex == 4) {
                        DigitalInputRemappingController.finishConfigureBinding_suspendRedirectMode()
                    } else if (bindingTypeComboBox.currentIndex == 5) {
                        DigitalInputRemappingController.finishConfigureBinding_toggleTouchpadEmulationFix()
                    }
                    goBack()
                }
            }
        }

        Component.onCompleted: {
        }

        Connections {
            target: DigitalInputRemappingController
        }

    }

}
