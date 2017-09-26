import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import matzman666.inputemulator 1.0

MyStackViewPage {
    id: deviceDigitalInputRemappingPage

    headerText: "Digital Input Settings "

    property int deviceIndex: -1

    property int buttonId: -1

    function setDeviceIndex(index, button) {
        deviceIndex = index
        buttonId = button
        updateGui()
    }

    function updateGui() {
        touchAsClickButton.checked = DigitalInputRemappingController.touchAsClickEnabled()
        normalBindingConfigButton.text = DigitalInputRemappingController.getNormalBindingStatus()
        longPressToggleButton.checked = DigitalInputRemappingController.isLongPressEnabled()
        longPressThresholdSlider.value = DigitalInputRemappingController.getLongPressThreshold()
        longPressBindingConfigButton.text = DigitalInputRemappingController.getLongPressBindingStatus()
        longPressImmediateReleaseToggle.checked = DigitalInputRemappingController.isLongPressImmediateRelease()
        doublePressToggleButton.checked = DigitalInputRemappingController.isDoublePressEnabled()
        doublePressThresholdSlider.value = DigitalInputRemappingController.getDoublePressThreshold()
        doublePressBindingConfigButton.text = DigitalInputRemappingController.getDoublePressBindingStatus()
        doublePressImmediateReleaseToggle.checked = DigitalInputRemappingController.isDoublePressImmediateRelease()
    }

    function updateBindingStatus() {
        normalBindingConfigButton.text = DigitalInputRemappingController.getNormalBindingStatus()
        longPressBindingConfigButton.text = DigitalInputRemappingController.getLongPressBindingStatus()
        doublePressBindingConfigButton.text = DigitalInputRemappingController.getDoublePressBindingStatus()
    }

    content: ColumnLayout {
        spacing: 18

        ColumnLayout {
            spacing: 64


            ColumnLayout {
                spacing: 8
                RowLayout {
                    MyText {
                        Layout.preferredWidth: 300
                        text: "Normal Press:"
                    }
                    MyPushButton {
                        id: normalBindingConfigButton
                        Layout.fillWidth: true
                        text: "Configure"
                        onClicked: {
                            DigitalInputRemappingController.startConfigureNormalBinding()
                            deviceDigitalBindingPage.setDeviceIndex(deviceIndex, buttonId)
                            MyResources.playFocusChangedSound()
                            var res = mainView.push(deviceDigitalBindingPage)
                        }
                    }
                }
                MyToggleButton {
                    id: touchAsClickButton
                    text: "Touch as Click"
                }
            }

            ColumnLayout {
                spacing: 8

                RowLayout {
                    MyToggleButton {
                        Layout.preferredWidth: 300
                        id: longPressToggleButton
                        text: "Long Press:"
                        onCheckedChanged: {
                            longPressBindingConfigButton.enabled = checked
                            longPressThresholdLabel.enabled = checked
                            longPressThresholdMinusButton.enabled = checked
                            longPressThresholdPlusButton.enabled = checked
                            longPressThresholdSlider.enabled = checked
                            longPressThresholdText.enabled = checked
                            longPressImmediateReleaseToggle.enabled = checked
                        }
                    }
                    MyPushButton {
                        id: longPressBindingConfigButton
                        Layout.fillWidth: true
                        enabled: false
                        text: "Configure"
                        onClicked: {
                            DigitalInputRemappingController.startConfigureLongPressBinding()
                            deviceDigitalBindingPage.setDeviceIndex(deviceIndex, buttonId)
                            MyResources.playFocusChangedSound()
                            var res = mainView.push(deviceDigitalBindingPage)
                        }
                    }
                }

                RowLayout {
                    spacing: 16

                    MyText {
                        id: longPressThresholdLabel
                        text: "Long Press Threshold:"
                        Layout.preferredWidth: 350
                        Layout.rightMargin: 12
                        enabled: false
                    }

                    MyPushButton2 {
                        id: longPressThresholdMinusButton
                        enabled: false
                        text: "-"
                        Layout.preferredWidth: 40
                        onClicked: {
                            longPressThresholdSlider.value -= 100
                        }
                    }

                    MySlider {
                        id: longPressThresholdSlider
                        enabled: false
                        from: 100
                        to: 5000
                        stepSize: 100
                        value: 300
                        snapMode: Slider.SnapAlways
                        Layout.fillWidth: true
                        onPositionChanged: {
                            var val = this.from + ( this.position  * (this.to - this.from))
                            longPressThresholdText.text = val.toFixed(0) + " ms"
                        }
                        onValueChanged: {
                            longPressThresholdText.text = value.toFixed(0) + " ms"
                        }
                    }

                    MyPushButton2 {
                        id: longPressThresholdPlusButton
                        enabled: false
                        text: "+"
                        Layout.preferredWidth: 40
                        onClicked: {
                            longPressThresholdSlider.value += 100
                        }
                    }

                    MyTextField {
                        id: longPressThresholdText
                        enabled: false
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
                                longPressThresholdSlider.value = v
                            } else {
                                longPressThresholdText.text = longPressThresholdSlider.value.toFixed(0) + " ms"
                            }
                        }
                    }
                }

                MyToggleButton {
                    id: longPressImmediateReleaseToggle
                    enabled: false
                    text: "Immediate Key Release"
                }
            }

            ColumnLayout {
                spacing: 8

                RowLayout {
                    MyToggleButton {
                        id: doublePressToggleButton
                        Layout.preferredWidth: 300
                        text: "Double Press:"
                        onCheckedChanged: {
                            doublePressBindingConfigButton.enabled = checked
                            doublePressThresholdLabel.enabled = checked
                            doublePressThresholdMinusButton.enabled = checked
                            doublePressThresholdPlusButton.enabled = checked
                            doublePressThresholdSlider.enabled = checked
                            doublePressThresholdText.enabled = checked
                            doublePressImmediateReleaseToggle.enabled = checked
                        }
                    }
                    MyPushButton {
                        id: doublePressBindingConfigButton
                        Layout.fillWidth: true
                        enabled: false
                        text: "Configure"
                        onClicked: {
                            DigitalInputRemappingController.startConfigureDoublePressBinding()
                            deviceDigitalBindingPage.setDeviceIndex(deviceIndex, buttonId)
                            MyResources.playFocusChangedSound()
                            var res = mainView.push(deviceDigitalBindingPage)
                        }
                    }
                }

                RowLayout {
                    spacing: 16

                    MyText {
                        id: doublePressThresholdLabel
                        text: "Double Press Threshold:"
                        Layout.preferredWidth: 350
                        Layout.rightMargin: 12
                        enabled: false
                    }

                    MyPushButton2 {
                        id: doublePressThresholdMinusButton
                        text: "-"
                        Layout.preferredWidth: 40
                        enabled: false
                        onClicked: {
                            doublePressThresholdSlider.value -= 100
                        }
                    }

                    MySlider {
                        id: doublePressThresholdSlider
                        enabled: false
                        from: 100
                        to: 5000
                        stepSize: 100
                        value: 300
                        snapMode: Slider.SnapAlways
                        Layout.fillWidth: true
                        onPositionChanged: {
                            var val = this.from + ( this.position  * (this.to - this.from))
                            doublePressThresholdText.text = val.toFixed(0) + " ms"
                        }
                        onValueChanged: {
                            doublePressThresholdText.text = value.toFixed(0) + " ms"
                        }
                    }

                    MyPushButton2 {
                        id: doublePressThresholdPlusButton
                        text: "+"
                        Layout.preferredWidth: 40
                        enabled: false
                        onClicked: {
                            doublePressThresholdSlider.value += 100
                        }
                    }

                    MyTextField {
                        id: doublePressThresholdText
                        text: "0.0"
                        Layout.preferredWidth: 150
                        Layout.leftMargin: 10
                        enabled: false
                        horizontalAlignment: Text.AlignHCenter
                        function onInputEvent(input) {
                            var val = parseFloat(input)
                            if (!isNaN(val)) {
                                if (val < 0) {
                                    val = 0
                                }
                                var v = val.toFixed(0)
                                doublePressThresholdSlider.value = v
                            } else {
                                doublePressThresholdText.text = doublePressThresholdSlider.value.toFixed(0) + " ms"
                            }
                        }
                    }
                }

                MyToggleButton {
                    id: doublePressImmediateReleaseToggle
                    enabled: false
                    text: "Immediate Key Release"
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
                    DeviceManipulationTabController.finishConfigureDigitalInputRemapping(
                        deviceIndex, buttonId,
                        touchAsClickButton.checked,
                        longPressToggleButton.checked,
                        longPressThresholdSlider.value,
                        longPressImmediateReleaseToggle.checked,
                        doublePressToggleButton.checked,
                        doublePressThresholdSlider.value,
                        doublePressImmediateReleaseToggle.checked
                    )
                    goBack()
                }
            }
        }

        Component.onCompleted: {
        }

        Connections {
            target: DigitalInputRemappingController
            onConfigureDigitalBindingFinished: {
                updateBindingStatus()
            }
        }

    }

}
