import QtQuick 2.9
import QtQuick.Controls 2.2
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
                        from: 1
                        to: 20
                        stepSize: 1
                        value: 2
                        Layout.fillWidth: true
                        onPositionChanged: {
                            var val = (this.from + this.position * (this.to-this.from)).toFixed(0)
                            movingAverageWindowText.text = val
                        }
                        onValueChanged: {
                            DeviceManipulationTabController.setMotionCompensationMovingAverageWindow(value.toFixed(0), false)
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
                            if (!isNaN(val) && val >= 0.0) {
                                DeviceManipulationTabController.setMotionCompensationMovingAverageWindow(val.toFixed(0))
                            } else {
                                movingAverageWindowText.text = DeviceManipulationTabController.getMotionCompensationMovingAverageWindow().toFixed(0)
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
            deviceSelectionComboBox.currentIndex = DeviceManipulationTabController.getMotionCompensationVelAccMode()
            kalmanProcessNoiseInputField.text = DeviceManipulationTabController.getMotionCompensationKalmanProcessNoise().toFixed(2)
            kalmanObservationNoiseInputField.text = DeviceManipulationTabController.getMotionCompensationKalmanObservationNoise().toFixed(2)
            movingAverageWindowSlider.value = DeviceManipulationTabController.getMotionCompensationMovingAverageWindow().toFixed(0)
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
