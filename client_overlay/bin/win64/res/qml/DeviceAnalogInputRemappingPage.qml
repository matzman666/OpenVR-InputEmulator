import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import matzman666.inputemulator 1.0

MyStackViewPage {
    id: deviceAnalogInputRemappingPage

    headerText: "Analog Input Settings "

    property int deviceIndex: -1

    property int axisId: -1

    property var _controllerIds: []

    function setDeviceIndex(index, axis) {
        deviceIndex = index
        axisId = axis
        var _controllerNames = ["<No Change>"]
        _controllerIds = []
        for (var i = 0; i < DeviceManipulationTabController.getDeviceCount(); i++) {
            var deviceId = DeviceManipulationTabController.getDeviceId(i)
            var deviceName = deviceId.toString() + ": " + DeviceManipulationTabController.getDeviceSerial(i)
            _controllerNames.push(deviceName)
            _controllerIds.push(DeviceManipulationTabController.getDeviceId(i))
        }
        openVRControllerComboBox.model = _controllerNames
        var _axisNames = []
        for (var i = 0; i < AnalogInputRemappingController.getAxisMaxCount(); i++) {
            _axisNames.push(AnalogInputRemappingController.getAxisName(i, true))
        }
        openvrAxisComboBox.model = _axisNames
        bindingTypeComboBox.currentIndex = AnalogInputRemappingController.getBindingType()
        if (bindingTypeComboBox.currentIndex == 0) {
            openVRControllerComboBox.currentIndex = 0
            openvrAxisComboBox.currentIndex = 0
            openvrXAxisInvertToggle.checked = false
            openvrYAxisInvertToggle.checked = false
            openvrAxisSwapToggle.checked = false
            deadzonesRangeSlider.first.value = 0.0
            deadzonesRangeSlider.second.value = 1.0
            touchpadEmulationComboBox.currentIndex = AnalogInputRemappingController.getBindingTouchpadEmulationMode()
            buttonDeadzoneFixToggle.checked = AnalogInputRemappingController.getBindingButtonPressDeadzoneFix()
        } else if (bindingTypeComboBox.currentIndex == 2) {
            var controllerId = AnalogInputRemappingController.getBindingOpenVRControllerId()
            var index = 0
            for (var i = 0; i < _controllerIds.length; i++) {
                if (controllerId == _controllerIds[i]) {
                    index = i + 1
                    break
                }

            }
            openVRControllerComboBox.currentIndex = index
            openvrAxisComboBox.currentIndex = AnalogInputRemappingController.getBindingOpenVRAxisId()
            openvrXAxisInvertToggle.checked = AnalogInputRemappingController.isBindingOpenVRXInverted()
            openvrYAxisInvertToggle.checked = AnalogInputRemappingController.isBindingOpenVRYInverted()
            openvrAxisSwapToggle.checked = AnalogInputRemappingController.isBindingOpenVRAxesSwapped()
            deadzonesRangeSlider.first.value = AnalogInputRemappingController.getBindingDeadzoneLower()
            deadzonesRangeSlider.second.value = AnalogInputRemappingController.getBindingDeadzoneUpper()
            touchpadEmulationComboBox.currentIndex = AnalogInputRemappingController.getBindingTouchpadEmulationMode()
            buttonDeadzoneFixToggle.checked = AnalogInputRemappingController.getBindingButtonPressDeadzoneFix()
        } else {
            openVRControllerComboBox.currentIndex = 0
            openvrAxisComboBox.currentIndex = 0
            openvrXAxisInvertToggle.checked = false
            openvrYAxisInvertToggle.checked = false
            openvrAxisSwapToggle.checked = false
            deadzonesRangeSlider.first.value = 0.0
            deadzonesRangeSlider.second.value = 1.0
            touchpadEmulationComboBox.currentIndex = 0
            buttonDeadzoneFixToggle.checked = false
        }
    }

    content: ColumnLayout {
            ColumnLayout {
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
                        "OpenVR"
                    ]
                    onCurrentIndexChanged: {
                        if (currentIndex == 0) {
                            openvrConfigContainer.visible = false
                            deadzoneConfigContainer.visible = false
                            touchpadEmulationContainer.visible = true
                        } else if (currentIndex == 2) {
                            openvrConfigContainer.visible = true
                            deadzoneConfigContainer.visible = true
                            touchpadEmulationContainer.visible = true
                        } else {
                            openvrConfigContainer.visible = false
                            deadzoneConfigContainer.visible = false
                            touchpadEmulationContainer.visible = false
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
                        model: []
                        onCurrentIndexChanged: {
                        }
                    }
                }

                RowLayout {
                    MyText {
                        Layout.preferredWidth: 200
                        text: "Axis:"
                    }
                    MyComboBox {
                        id: openvrAxisComboBox
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
                        id: openvrXAxisInvertToggle
                        Layout.preferredWidth: 300
                        text: "Invert X Axis"
                    }
                    MyToggleButton {
                        id: openvrYAxisInvertToggle
                        Layout.preferredWidth: 300
                        text: "Invert Y Axis"
                    }
                    MyToggleButton {
                        id: openvrAxisSwapToggle
                        Layout.preferredWidth: 300
                        text: "Swap X/Y"
                    }
                }
            }

            ColumnLayout {
                id: deadzoneConfigContainer
                visible: false
                spacing: 18

                RowLayout {
                    spacing: 16

                    MyText {
                        text: "Dead Zone:"
                        Layout.preferredWidth: 180
                        Layout.rightMargin: 12
                    }

                    MyTextField {
                        id: lowerDeadzonesText
                        text: "0.0"
                        Layout.preferredWidth: 150
                        Layout.rightMargin: 10
                        horizontalAlignment: Text.AlignHCenter
                        function onInputEvent(input) {
                            var val = parseFloat(input)
                            if (!isNaN(val)) {
                                if (val < 0.0) {
                                    val = 0.0
                                } else if (val > 1.0) {
                                    val = 1.0
                                }
                                var v = val.toFixed(2)
                                deadzonesRangeSlider.first.value = v
                            } else {
                                lowerDeadzonesText.text = deadzonesRangeSlider.first.value.toFixed(2)
                            }
                        }
                    }
                    MyRangeSlider {
                        id: deadzonesRangeSlider
                        from: 0.0
                        to: 1.0
                        stepSize: 0.01
                        first.value: 0.0
                        second.value: 1.0
                        Layout.fillWidth: true
                        first.onValueChanged: {
                            lowerDeadzonesText.text = first.value.toFixed(2)
                        }
                        second.onValueChanged: {
                            upperDeadzonesText.text = second.value.toFixed(2)
                        }
                    }

                    MyTextField {
                        id: upperDeadzonesText
                        text: "1.0"
                        Layout.preferredWidth: 150
                        Layout.leftMargin: 10
                        horizontalAlignment: Text.AlignHCenter
                        function onInputEvent(input) {
                            var val = parseFloat(input)
                            if (!isNaN(val)) {
                                if (val < 0.0) {
                                    val = 0.0
                                } else if (val > 1.0) {
                                    val = 1.0
                                }
                                var v = val.toFixed(2)
                                deadzonesRangeSlider.second.value = v
                            } else {
                                upperDeadzonesText.text = deadzonesRangeSlider.second.value.toFixed(2)
                            }
                        }
                    }
                }
            }
        }

        ColumnLayout {
            id: touchpadEmulationContainer
            spacing: 4
            Layout.topMargin: 64

            RowLayout {
                MyText {
                    Layout.preferredWidth: 370
                    text: "Joystick Touchpad Emulation:"
                }
                MyComboBox {
                    id: touchpadEmulationComboBox
                    Layout.maximumWidth: 480
                    Layout.minimumWidth: 480
                    Layout.preferredWidth: 480
                    Layout.fillWidth: true
                    model: [
                        "Disabled",
                        "Position Based",
                        "Position Based (Deferred Zero Update)"
                    ]
                }
            }

            MyToggleButton {
                id: buttonDeadzoneFixToggle
                text: "Button Press Deadzone Fix"
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
                    var touchpadEmulation = touchpadEmulationComboBox.currentIndex
                    var deadzoneFix = buttonDeadzoneFixToggle.checked
                    if (bindingTypeComboBox.currentIndex == 0) {
                        AnalogInputRemappingController.finishConfigure_Original(touchpadEmulation, deadzoneFix)
                    } else if (bindingTypeComboBox.currentIndex == 1) {
                        AnalogInputRemappingController.finishConfigure_Disabled()
                    } else if (bindingTypeComboBox.currentIndex == 2) {
                        var controllerId = -1
                        if (openVRControllerComboBox.currentIndex > 0) {
                            var controllerId = _controllerIds[openVRControllerComboBox.currentIndex - 1]
                        }
                        var mappedAxisId = openvrAxisComboBox.currentIndex
                        var invertXAxis = openvrXAxisInvertToggle.checked
                        var invertYAxis = openvrYAxisInvertToggle.checked
                        var swapAxes = openvrAxisSwapToggle.checked
                        var lowerDeadzone = deadzonesRangeSlider.first.value
                        var upperDeadzone = deadzonesRangeSlider.second.value
                        AnalogInputRemappingController.finishConfigure_OpenVR(controllerId, mappedAxisId, invertXAxis, invertYAxis, swapAxes, lowerDeadzone, upperDeadzone, touchpadEmulation, deadzoneFix)
                    }
                    goBack()
                }
            }
        }


        Component.onCompleted: {
        }

    }

}
