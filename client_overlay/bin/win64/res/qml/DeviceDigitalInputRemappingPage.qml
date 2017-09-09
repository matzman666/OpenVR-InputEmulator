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
        normalBindingStatusText.text = DigitalInputRemappingController.getNormalBindingStatus()
        longPressToggleButton.checked = DigitalInputRemappingController.isLongPressEnabled()
        longPressThresholdSlider.value = DigitalInputRemappingController.getLongPressThreshold()
        longPressBindingStatusText.text = DigitalInputRemappingController.getLongPressBindingStatus()
        doublePressToggleButton.checked = DigitalInputRemappingController.isDoublePressEnabled()
        doublePressThresholdSlider.value = DigitalInputRemappingController.getDoublePressThreshold()
        doublePressBindingStatusText.text = DigitalInputRemappingController.getDoublePressBindingStatus()
    }

    function updateBindingStatus() {
        normalBindingStatusText.text = DigitalInputRemappingController.getNormalBindingStatus()
        longPressBindingStatusText.text = DigitalInputRemappingController.getLongPressBindingStatus()
        doublePressBindingStatusText.text = DigitalInputRemappingController.getDoublePressBindingStatus()
    }

    onPageBackButtonClicked: {
        var touchAsClick = touchAsClickButton.checked
        var longPress = longPressToggleButton.checked
        var longPressThreshold = longPressThresholdSlider.value
        var doublePress = doublePressToggleButton.checked
        var doublePressThreshold = doublePressThresholdSlider.value
        DeviceManipulationTabController.finishConfigureDigitalInputRemapping(deviceIndex, buttonId, touchAsClick, longPress, longPressThreshold, doublePress, doublePressThreshold)
    }

    content: ColumnLayout {
        spacing: 18

        ColumnLayout {
            spacing: 64


            ColumnLayout {
                Layout.topMargin: 32
                spacing: 16
                RowLayout {
                    MyText {
                        Layout.preferredWidth: 270
                        text: "Binding:"
                    }
                    MyText {
                        id: normalBindingStatusText
                        Layout.preferredWidth: 650
                        text: "<Status>"
                    }
                    MyPushButton {
                        Layout.preferredWidth: 150
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

                RowLayout {
                    MyToggleButton {
                        Layout.preferredWidth: 270
                        id: longPressToggleButton
                        text: "Long Press:"
                    }
                    MyText {
                        id: longPressBindingStatusText
                        Layout.preferredWidth: 650
                        text: "<Status>"
                    }
                    MyPushButton {
                        Layout.preferredWidth: 150
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
                        text: "Long Press Threshold:"
                        Layout.preferredWidth: 350
                        Layout.rightMargin: 12
                    }

                    MyPushButton2 {
                        text: "-"
                        Layout.preferredWidth: 40
                        onClicked: {
                            longPressThresholdSlider.value -= 100
                        }
                    }

                    MySlider {
                        id: longPressThresholdSlider
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
                        text: "+"
                        Layout.preferredWidth: 40
                        onClicked: {
                            longPressThresholdSlider.value += 100
                        }
                    }

                    MyTextField {
                        id: longPressThresholdText
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
            }

            ColumnLayout {

                RowLayout {
                    MyToggleButton {
                        id: doublePressToggleButton
                        Layout.preferredWidth: 270
                        text: "Double Press:"
                    }
                    MyText {
                        id: doublePressBindingStatusText
                        Layout.preferredWidth: 650
                        text: "<Status>"
                    }
                    MyPushButton {
                        Layout.preferredWidth: 150
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
                        text: "Double Press Threshold:"
                        Layout.preferredWidth: 350
                        Layout.rightMargin: 12
                    }

                    MyPushButton2 {
                        text: "-"
                        Layout.preferredWidth: 40
                        onClicked: {
                            doublePressThresholdSlider.value -= 100
                        }
                    }

                    MySlider {
                        id: doublePressThresholdSlider
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
                        text: "+"
                        Layout.preferredWidth: 40
                        onClicked: {
                            doublePressThresholdSlider.value += 100
                        }
                    }

                    MyTextField {
                        id: doublePressThresholdText
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
                                doublePressThresholdSlider.value = v
                            } else {
                                doublePressThresholdText.text = doublePressThresholdSlider.value.toFixed(0) + " ms"
                            }
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
            onConfigureDigitalBindingFinished: {
                updateBindingStatus()
            }
        }

    }

}
