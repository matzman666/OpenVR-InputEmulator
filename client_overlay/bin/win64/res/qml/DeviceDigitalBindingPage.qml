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
            _controllerNames.push(DeviceManipulationTabController.getDeviceSerial(i))
            _controllerIds.push(DeviceManipulationTabController.getDeviceId(i))
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
            keyboardKeyComboBox.currentIndex = DigitalInputRemappingController.keyboardKeyIndex()
            toggleModeToggle.checked = DigitalInputRemappingController.isToggleModeEnabled()
            toggleThresholdSlider.value = DigitalInputRemappingController.toggleModeThreshold()
            autoTriggerToggle.checked = DigitalInputRemappingController.isAutoTriggerEnabled()
            triggerFrequencySlider.value = (DigitalInputRemappingController.autoTriggerFrequency() / 100).toFixed(1)
        } else {
            openVRControllerComboBox.currentIndex = 0
            openvrButtonComboBox.currentIndex = 0
            toggleModeToggle.checked = false
            toggleThresholdSlider.value = 300
            autoTriggerToggle.checked = false
            triggerFrequencySlider.value = 10
        }
    }

    onPageBackButtonClicked: {
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
            DigitalInputRemappingController.finishConfigureBinding_keyboard(shift, ctrl, alt, keyIndex, toggleMode, toggleDelay, autoTrigger, autoTriggerFreq)
        } else if (bindingTypeComboBox.currentIndex == 4) {
            DigitalInputRemappingController.finishConfigureBinding_suspendRedirectMode()
        }
    }

    content: ColumnLayout {
        spacing: 64

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
                    "Suspend Redirect Mode"
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
            visible: false
            spacing: 18

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
            visible: false
            spacing: 18
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
                    Layout.preferredWidth: 200
                    text: "Shift"
                }
                MyToggleButton {
                    id: keyboardCtrlToggle
                    Layout.preferredWidth: 200
                    text: "CTRL"
                }
                MyToggleButton {
                    id: keyboardAltToggle
                    Layout.preferredWidth: 200
                    text: "Alt"
                }
            }
        }

        ColumnLayout {
            id: toggleModeContainer
            visible: false
            spacing: 18

            MyToggleButton {
                id: toggleModeToggle
                text: "Enable Toggle Mode"
            }

            RowLayout {
                spacing: 16

                MyText {
                    text: "Toggle Threshold:"
                    Layout.preferredWidth: 350
                    Layout.rightMargin: 12
                }

                MyPushButton2 {
                    text: "-"
                    Layout.preferredWidth: 40
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
                    onPositionChanged: {
                        var val = this.from + ( this.position  * (this.to - this.from))
                        toggleThresholdText.text = val.toFixed(0) + " ms"
                    }
                    onValueChanged: {
                        toggleThresholdText.text = value.toFixed(0) + " ms"
                    }
                }

                MyPushButton2 {
                    text: "+"
                    Layout.preferredWidth: 40
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
            visible: false
            spacing: 18

            MyToggleButton {
                id: autoTriggerToggle
                text: "Enable Auto-Trigger"
            }

            RowLayout {
                spacing: 16

                MyText {
                    text: "Trigger Frequency:"
                    Layout.preferredWidth: 350
                    Layout.rightMargin: 12
                }

                MyPushButton2 {
                    text: "-"
                    Layout.preferredWidth: 40
                    onClicked: {
                        triggerFrequencySlider.value -= 0.1
                    }
                }

                MySlider {
                    id: triggerFrequencySlider
                    from: 0.1
                    to: 90.0
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
                    text: "+"
                    Layout.preferredWidth: 40
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


        Component.onCompleted: {
        }

        Connections {
            target: DigitalInputRemappingController
        }

    }

}
