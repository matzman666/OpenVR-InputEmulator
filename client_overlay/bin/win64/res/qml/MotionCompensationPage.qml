import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import matzman666.inputemulator 1.0

MyStackViewPage {
    id: motionCompensationPage

    headerText: "Motion Compensation Settings"

    property bool setupFinished: false

    content: ColumnLayout {
        spacing: 18

        RowLayout {
            spacing: 18

            MyText {
                text: "Vel/Acc Compensation Mode:"
            }

            MyComboBox {
                id: deviceSelectionComboBox
                Layout.maximumWidth: 750
                Layout.minimumWidth: 750
                Layout.preferredWidth: 750
                Layout.fillWidth: true
                model: [
                    "Disabled",
                    "Set Zero",
                    "Use Reference Tracker",
                    "Linear Approximation w/ Moving Average",
                    "Kalman Filter"
                ]
                onCurrentIndexChanged: {
                    if (setupFinished) {
                        DeviceManipulationTabController.setMotionCompensationVelAccMode(currentIndex)
                    }
                    if (currentIndex == 4) {
                        kalmanFilterParameterBox.visible = true
                        linearApproximationParameterBox.visible = false
                    } else if (currentIndex == 3) {
                        kalmanFilterParameterBox.visible = false
                        linearApproximationParameterBox.visible = true
                    } else {
                        kalmanFilterParameterBox.visible = false
                        linearApproximationParameterBox.visible = false
                    }
                }
            }
        }

        GroupBox {
            id: kalmanFilterParameterBox
            Layout.fillWidth: true


            label: MyText {
                leftPadding: 10
                text: "Kalman Filter Parameters"
                bottomPadding: -10
                color: kalmanFilterParameterBox.enabled ? "white" : "gray"
            }

            background: Rectangle {
                color: "transparent"
                border.color: kalmanFilterParameterBox.enabled ? "white" : "gray"
                radius: 8
            }

            ColumnLayout {
                anchors.fill: parent

                Rectangle {
                    color: kalmanFilterParameterBox.enabled ? "white" : "gray"
                    height: 1
                    Layout.fillWidth: true
                    Layout.bottomMargin: 5
                }

                RowLayout {
                    spacing: 18

                    MyText {
                        text: "Process Noise:"
                        color: kalmanFilterParameterBox.enabled ? "white" : "gray"
                    }

                    MyTextField {
                        id: kalmanProcessNoiseInputField
                        color: kalmanFilterParameterBox.enabled ? "white" : "gray"
                        text: "0.00"
                        Layout.preferredWidth: 140
                        Layout.leftMargin: 10
                        Layout.rightMargin: 10
                        horizontalAlignment: Text.AlignHCenter
                        function onInputEvent(input) {
                            var val = parseFloat(input)
                            if (!isNaN(val) && val >= 0.0) {
                                DeviceManipulationTabController.setMotionCompensationKalmanProcessNoise(val.toFixed(2))
                            } else {
                                kalmanProcessNoiseInputField.text = DeviceManipulationTabController.getMotionCompensationKalmanProcessNoise().toFixed(2)
                            }
                        }
                    }

                    Item {
                        width: 100
                    }

                    MyText {
                        text: "Observation Noise:"
                        color: kalmanFilterParameterBox.enabled ? "white" : "gray"
                    }

                    MyTextField {
                        id: kalmanObservationNoiseInputField
                        color: kalmanFilterParameterBox.enabled ? "white" : "gray"
                        text: "0.00"
                        Layout.preferredWidth: 140
                        Layout.leftMargin: 10
                        Layout.rightMargin: 10
                        horizontalAlignment: Text.AlignHCenter
                        function onInputEvent(input) {
                            var val = parseFloat(input)
                            if (!isNaN(val) && val >= 0.0) {
                                DeviceManipulationTabController.setMotionCompensationKalmanObservationNoise(val.toFixed(2))
                            } else {
                                kalmanObservationNoiseInputField.text = DeviceManipulationTabController.getMotionCompensationKalmanObservationNoise().toFixed(2)
                            }
                        }
                    }
                }
            }
        }

        GroupBox {
            id: linearApproximationParameterBox
            Layout.fillWidth: true


            label: MyText {
                leftPadding: 10
                text: "Linear Approximation Parameters"
                bottomPadding: -10
                color: linearApproximationParameterBox.enabled ? "white" : "gray"
            }

            background: Rectangle {
                color: "transparent"
                border.color: linearApproximationParameterBox.enabled ? "white" : "gray"
                radius: 8
            }

            ColumnLayout {
                anchors.fill: parent

                Rectangle {
                    color: linearApproximationParameterBox.enabled ? "white" : "gray"
                    height: 1
                    Layout.fillWidth: true
                    Layout.bottomMargin: 5
                }

                RowLayout {
                    spacing: 18

                    MyText {
                        text: "Moving Average Window:"
                        color: linearApproximationParameterBox.enabled ? "white" : "gray"
                    }

                    MyPushButton2 {
                        text: "-"
                        Layout.preferredWidth: 40
                        onClicked: {
                            movingAverageWindowSlider.decrease()
                        }
                    }

                    MySlider {
                        id: movingAverageWindowSlider
                        // There seems to be a bug which causes the handle to snap to the wrong position when 'from' is not 0
                        from: 0
                        to: 19
                        stepSize: 1
                        value: 2
                        Layout.fillWidth: true
                        onPositionChanged: {
                            var val = (1 + ( this.position  * this.to)).toFixed(0)
                            if (activeFocus) {
                                //ChaperoneTabController.setHeight(val, false);
                            }
                            movingAverageWindowText.text = val
                        }
                        onValueChanged: {
                            //ChaperoneTabController.setHeight(value.toFixed(0), false)
                        }
                    }

                    MyPushButton2 {
                        text: "+"
                        Layout.preferredWidth: 40
                        onClicked: {
                            movingAverageWindowSlider.increase()
                        }
                    }

                    MyTextField {
                        id: movingAverageWindowText
                        text: "0.00"
                        Layout.preferredWidth: 100
                        Layout.leftMargin: 10
                        horizontalAlignment: Text.AlignHCenter
                        function onInputEvent(input) {
                            var val = parseFloat(input)
                            if (!isNaN(val)) {
                                if (val < 1) {
                                    val = 1
                                }
                                var v = val.toFixed(0) - 1
                                if (v <= movingAverageWindowSlider.to) {
                                    movingAverageWindowSlider.value = v
                                } else {
                                    //ChaperoneTabController.setHeight(v, false)
                                }
                            }
                            //text = ChaperoneTabController.height.toFixed(2)
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
            deviceSelectionComboBox.currentIndex = DeviceManipulationTabController.getMotionCompensationVelAccMode()
            kalmanProcessNoiseInputField.text = DeviceManipulationTabController.getMotionCompensationKalmanProcessNoise().toFixed(2)
            kalmanObservationNoiseInputField.text = DeviceManipulationTabController.getMotionCompensationKalmanObservationNoise().toFixed(2)
            setupFinished = true
        }

        Connections {
            target: DeviceManipulationTabController
            onMotionCompensationVelAccModeChanged: {
                deviceSelectionComboBox.currentIndex = mode
            }
            onMotionCompensationKalmanProcessNoiseChanged: {
                kalmanProcessNoiseInputField.text = DeviceManipulationTabController.getMotionCompensationKalmanProcessNoise().toFixed(2)
            }
            onMotionCompensationKalmanObservationNoiseChanged: {
                kalmanObservationNoiseInputField.text = DeviceManipulationTabController.getMotionCompensationKalmanObservationNoise().toFixed(2)
            }
        }

    }

}
