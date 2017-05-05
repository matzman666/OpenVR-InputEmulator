import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import matzman666.inputemulator 1.0

MyStackViewPage {
    id: motionCompensationPage

    headerText: "Motion Compensation Settings"

    property double centerX: 0.0
    property double centerY: 0.0
    property double centerZ: 0.0
    property double centerStep: 0.1

    function updateValues() {
        centerPointModeSelectionComboBox.currentIndex = DeviceManipulationTabController.getMotionCompensationCenterMode()
        centerX = DeviceManipulationTabController.getMotionCompensationCenter(0).toFixed(3)
        centerY = DeviceManipulationTabController.getMotionCompensationCenter(1).toFixed(3)
        centerZ = DeviceManipulationTabController.getMotionCompensationCenter(2).toFixed(3)
        updateCenterTextFields()
        checkCoordButtons()
    }

    function updateCenterTextFields() {
        xInputField.text = centerX.toFixed(3)
        yInputField.text = centerY.toFixed(3)
        zInputField.text = centerZ.toFixed(3)
    }

    function checkCoordButtons() {
        if (centerPointModeSelectionComboBox.currentIndex == 0 || centerPointModeSelectionComboBox.currentIndex == 2) {
            coordRightControllerButton.enabled = true
            coordLeftControllerButton.enabled = true
            coordSittingUniverseButton.enabled = true
        } else {
            coordRightControllerButton.enabled = false
            coordLeftControllerButton.enabled = false
            coordSittingUniverseButton.enabled = false
        }
    }

    content: ColumnLayout {
        spacing: 18

        GroupBox {
            Layout.fillWidth: true

            label: MyText {
                leftPadding: 10
                text: "Center Point"
                bottomPadding: -10
            }

            background: Rectangle {
                color: "transparent"
                border.color: "#ffffff"
                radius: 8
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 18


                Rectangle {
                    color: "#ffffff"
                    height: 1
                    Layout.fillWidth: true
                    Layout.bottomMargin: 5
                }

                RowLayout {
                    spacing: 18

                    MyText {
                        text: "Mode:"
                    }

                    MyComboBox {
                        id: centerPointModeSelectionComboBox
                        Layout.maximumWidth: 999
                        Layout.minimumWidth: 999
                        Layout.preferredWidth: 999
                        Layout.fillWidth: true
                        model: [
                            "Absolute",
                            "Relative",
                            /*"Absolute (Raw Universe)",
                            "Relative (Raw Universe)",*/
                        ]
                        onCurrentIndexChanged: {
                            checkCoordButtons()
                        }
                    }
                }

                RowLayout {

                    MyText {
                        text: "X:"
                        horizontalAlignment: Text.AlignRight
                        Layout.preferredWidth: 80
                        Layout.rightMargin: 12
                    }

                    MyPushButton2 {
                        id: xMinusButton
                        Layout.preferredWidth: 40
                        text: "-"
                        onClicked: {
                            centerX -= centerStep
                            updateCenterTextFields()
                        }
                    }

                    MyTextField {
                        id: xInputField
                        text: "0.00"
                        keyBoardUID: 301
                        Layout.preferredWidth: 140
                        Layout.leftMargin: 10
                        Layout.rightMargin: 10
                        horizontalAlignment: Text.AlignHCenter
                        function onInputEvent(input) {
                            var val = parseFloat(input)
                            if (!isNaN(val)) {
                                centerX = val.toFixed(3)
                            }
                            updateCenterTextFields()
                        }
                    }

                    MyPushButton2 {
                        id: xPlusButton
                        Layout.preferredWidth: 40
                        text: "+"
                        onClicked: {
                            centerX += centerStep
                            updateCenterTextFields()
                        }
                    }

                    MyText {
                        text: "Y:"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        Layout.leftMargin: 12
                        Layout.rightMargin: 12
                    }

                    MyPushButton2 {
                        id: yMinusButton
                        Layout.preferredWidth: 40
                        text: "-"
                        onClicked: {
                            centerY -= centerStep
                            updateCenterTextFields()
                        }
                    }

                    MyTextField {
                        id: yInputField
                        text: "0.00"
                        keyBoardUID: 302
                        Layout.preferredWidth: 140
                        Layout.leftMargin: 10
                        Layout.rightMargin: 10
                        horizontalAlignment: Text.AlignHCenter
                        function onInputEvent(input) {
                            var val = parseFloat(input)
                            if (!isNaN(val)) {
                                centerY = val.toFixed(3)
                            }
                            updateCenterTextFields()
                        }
                    }

                    MyPushButton2 {
                        id: yPlusButton
                        Layout.preferredWidth: 40
                        text: "+"
                        onClicked: {
                            centerY += centerStep
                            updateCenterTextFields()
                        }
                    }

                    MyText {
                        text: "Z:"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        Layout.leftMargin: 12
                        Layout.rightMargin: 12
                    }

                    MyPushButton2 {
                        id: zMinusButton
                        Layout.preferredWidth: 40
                        text: "-"
                        onClicked: {
                            centerZ -= centerStep
                            updateCenterTextFields()
                        }
                    }

                    MyTextField {
                        id: zInputField
                        text: "0.00"
                        keyBoardUID: 303
                        Layout.preferredWidth: 140
                        Layout.leftMargin: 10
                        Layout.rightMargin: 10
                        horizontalAlignment: Text.AlignHCenter
                        function onInputEvent(input) {
                            var val = parseFloat(input)
                            if (!isNaN(val)) {
                                centerZ = val.toFixed(3)
                            }
                            updateCenterTextFields()
                        }
                    }

                    MyPushButton2 {
                        id: zPlusButton
                        Layout.preferredWidth: 40
                        text: "+"
                        onClicked: {
                            centerZ += centerStep
                            updateCenterTextFields()
                        }
                    }
                }

                RowLayout {
                    spacing: 18

                    MyPushButton {
                        id: centerPointApplyButton
                        Layout.preferredWidth: 300
                        text: "Apply"
                        onClicked: {
                            DeviceManipulationTabController.setMotionCompensationCenter(centerPointModeSelectionComboBox.currentIndex, centerX, centerY, centerZ)
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    MyPushButton {
                        id: centerPointResetButton
                        Layout.preferredWidth: 300
                        text: "Reset"
                        onClicked: {
                            updateValues()
                        }
                    }
                }
            }
        }

        MyPushButton {
            id: coordRightControllerButton
            Layout.preferredWidth: 600
            text: "Retrieve coordinates of right controller"
            onClicked: {
                DeviceManipulationTabController.retrieveMotionCompensationTmpCenter(centerPointModeSelectionComboBox.currentIndex, 0)
                centerX = DeviceManipulationTabController.getMotionCompensationTmpCenter(0).toFixed(3)
                centerY = DeviceManipulationTabController.getMotionCompensationTmpCenter(1).toFixed(3)
                centerZ = DeviceManipulationTabController.getMotionCompensationTmpCenter(2).toFixed(3)
                updateCenterTextFields()
            }
        }

        MyPushButton {
            id: coordLeftControllerButton
            Layout.preferredWidth: 600
            text: "Retrieve coordinates of left controller"
            onClicked: {
                DeviceManipulationTabController.retrieveMotionCompensationTmpCenter(centerPointModeSelectionComboBox.currentIndex, 1)
                centerX = DeviceManipulationTabController.getMotionCompensationTmpCenter(0).toFixed(3)
                centerY = DeviceManipulationTabController.getMotionCompensationTmpCenter(1).toFixed(3)
                centerZ = DeviceManipulationTabController.getMotionCompensationTmpCenter(2).toFixed(3)
                updateCenterTextFields()
            }
        }

        MyPushButton {
            id: coordSittingUniverseButton
            Layout.preferredWidth: 600
            text: "Retrieve coordinates of sitting universe center"
            onClicked: {
                DeviceManipulationTabController.retrieveMotionCompensationTmpCenter(centerPointModeSelectionComboBox.currentIndex, 2)
                centerX = DeviceManipulationTabController.getMotionCompensationTmpCenter(0).toFixed(3)
                centerY = DeviceManipulationTabController.getMotionCompensationTmpCenter(1).toFixed(3)
                centerZ = DeviceManipulationTabController.getMotionCompensationTmpCenter(2).toFixed(3)
                updateCenterTextFields()
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Connections {
            target: DeviceManipulationTabController
            onDeviceInfoChanged: {
            }
        }

    }

}
