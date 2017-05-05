import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

GroupBox {
    property string boxTitle: "Offsets"

    property double offsetYaw: 0.0
    property double offsetPitch: 0.0
    property double offsetRoll: 0.0

    property double offsetX: 0.0
    property double offsetY: 0.0
    property double offsetZ: 0.0

    property double offsetRotationStep: 5.0
    property double offsetTranslationStep: 1.0

    property int keyboardUIDBase: 200

    property var setTranslationOffset: function(x, y, z) {}
    property var setRotationOffset: function(yaw, pitch, roll) {}
    property var updateValues: function() {}

    function updateGUI() {
        yawInputField.text = offsetYaw.toFixed(1) + "°"
        pitchInputField.text = offsetPitch.toFixed(1) + "°"
        rollInputField.text = offsetRoll.toFixed(1) + "°"
        xInputField.text = offsetX.toFixed(1)
        yInputField.text = offsetY.toFixed(1)
        zInputField.text = offsetZ.toFixed(1)
    }

    Layout.fillWidth: true

    label: MyText {
        leftPadding: 10
        text: parent.boxTitle
        bottomPadding: -10
    }

    background: Rectangle {
        color: "transparent"
        border.color: "#ffffff"
        radius: 8
    }

    ColumnLayout {
        anchors.fill: parent

        Rectangle {
            color: "#ffffff"
            height: 1
            Layout.fillWidth: true
            Layout.bottomMargin: 5
        }

        GridLayout {
            columns: 12

            MyText {
                text: "Yaw:"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                Layout.rightMargin: 12
            }

            MyPushButton2 {
                id: yawMinusButton
                Layout.preferredWidth: 40
                text: "-"
                onClicked: {
                    var value = offsetYaw - offsetRotationStep
                    if (value < -180.0) {
                        value += 360.0
                    }
                    setRotationOffset(value, offsetPitch, offsetRoll)
                }
            }

            MyTextField {
                id: yawInputField
                text: "0.00"
                keyBoardUID: keyboardUIDBase
                Layout.preferredWidth: 140
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                horizontalAlignment: Text.AlignHCenter
                function onInputEvent(input) {
                    var val = parseFloat(input)
                    if (!isNaN(val)) {
                        if (val < -180.0) {
                            val = -180.0
                        } else if (val > 180.0) {
                            val = 180.0
                        }
                        setRotationOffset(val.toFixed(1), offsetPitch, offsetRoll)
                    } else {
                        getOffsets()
                    }
                }
            }

            MyPushButton2 {
                id: yawPlusButton
                Layout.preferredWidth: 40
                text: "+"
                onClicked: {
                    var value = offsetYaw + offsetRotationStep
                    if (value > 180.0) {
                        value -= 360.0
                    }
                    setRotationOffset(value, offsetPitch, offsetRoll)
                }
            }

            MyText {
                text: "Pitch:"
                horizontalAlignment: Text.AlignRight
                Layout.preferredWidth: 80
                Layout.leftMargin: 12
                Layout.rightMargin: 12
            }

            MyPushButton2 {
                id: pitchMinusButton
                Layout.preferredWidth: 40
                text: "-"
                onClicked: {
                    var value = offsetPitch - offsetRotationStep
                    if (value < -180.0) {
                        value += 360.0
                    }
                    setRotationOffset(offsetYaw, value, offsetRoll)
                }
            }

            MyTextField {
                id: pitchInputField
                text: "0.00"
                keyBoardUID: keyboardUIDBase + 1
                Layout.preferredWidth: 140
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                horizontalAlignment: Text.AlignHCenter
                function onInputEvent(input) {
                    var val = parseFloat(input)
                    if (!isNaN(val)) {
                        if (val < -180.0) {
                            val = -180.0
                        } else if (val > 180.0) {
                            val = 180.0
                        }
                        setRotationOffset(offsetYaw, val.toFixed(1), offsetRoll)
                    } else {
                        getOffsets()
                    }
                }
            }

            MyPushButton2 {
                id: pitchPlusButton
                Layout.preferredWidth: 40
                text: "+"
                onClicked: {
                    var value = offsetPitch + offsetRotationStep
                    if (value > 180.0) {
                        value -= 360.0
                    }
                    setRotationOffset(offsetYaw, value, offsetRoll)
                }
            }

            MyText {
                text: "Roll:"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                Layout.leftMargin: 12
                Layout.rightMargin: 12
            }

            MyPushButton2 {
                id: rollMinusButton
                Layout.preferredWidth: 40
                text: "-"
                onClicked: {
                    var value = offsetRoll - offsetRotationStep
                    if (value < -180.0) {
                        value += 360.0
                    }
                    setRotationOffset(offsetYaw, offsetPitch, value)
                }
            }

            MyTextField {
                id: rollInputField
                text: "0.00"
                keyBoardUID: keyboardUIDBase + 2
                Layout.preferredWidth: 140
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                horizontalAlignment: Text.AlignHCenter
                function onInputEvent(input) {
                    var val = parseFloat(input)
                    if (!isNaN(val)) {
                        if (val < -180.0) {
                            val = -180.0
                        } else if (val > 180.0) {
                            val = 180.0
                        }
                        setRotationOffset(offsetYaw, offsetPitch, val.toFixed(1))
                    } else {
                        getOffsets()
                    }
                }
            }

            MyPushButton2 {
                id: rollPlusButton
                Layout.preferredWidth: 40
                text: "+"
                onClicked: {
                    var value = offsetRoll + offsetRotationStep
                    if (value > 180.0) {
                        value -= 360.0
                    }
                    setRotationOffset(offsetYaw, offsetPitch, value)
                }
            }

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
                    var value = offsetX - offsetTranslationStep
                    setTranslationOffset(value, offsetY, offsetZ)
                }
            }

            MyTextField {
                id: xInputField
                text: "0.00"
                keyBoardUID: keyboardUIDBase + 3
                Layout.preferredWidth: 140
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                horizontalAlignment: Text.AlignHCenter
                function onInputEvent(input) {
                    var val = parseFloat(input)
                    if (!isNaN(val)) {
                        setTranslationOffset(val.toFixed(1), offsetY, offsetZ)
                    } else {
                        getOffsets()
                    }
                }
            }

            MyPushButton2 {
                id: xPlusButton
                Layout.preferredWidth: 40
                text: "+"
                onClicked: {
                    var value = offsetX + offsetTranslationStep
                    setTranslationOffset(value, offsetY, offsetZ)
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
                    var value = offsetY - offsetTranslationStep
                    setTranslationOffset(offsetX, value, offsetZ)
                }
            }

            MyTextField {
                id: yInputField
                text: "0.00"
                keyBoardUID: keyboardUIDBase + 4
                Layout.preferredWidth: 140
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                horizontalAlignment: Text.AlignHCenter
                function onInputEvent(input) {
                    var val = parseFloat(input)
                    if (!isNaN(val)) {
                        setTranslationOffset(offsetX, val.toFixed(1), offsetZ)
                    } else {
                        getOffsets()
                    }
                }
            }

            MyPushButton2 {
                id: yPlusButton
                Layout.preferredWidth: 40
                text: "+"
                onClicked: {
                    var value = offsetY + offsetTranslationStep
                    setTranslationOffset(offsetX, value, offsetZ)
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
                    var value = offsetZ - offsetTranslationStep
                    setTranslationOffset(offsetX, offsetY, value)
                }
            }

            MyTextField {
                id: zInputField
                text: "0.00"
                keyBoardUID: keyboardUIDBase + 5
                Layout.preferredWidth: 140
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                horizontalAlignment: Text.AlignHCenter
                function onInputEvent(input) {
                    var val = parseFloat(input)
                    if (!isNaN(val)) {
                        setTranslationOffset(offsetX, offsetY, val.toFixed(1))
                    } else {
                        getOffsets()
                    }
                }
            }

            MyPushButton2 {
                id: zPlusButton
                Layout.preferredWidth: 40
                text: "+"
                onClicked: {
                    var value = offsetZ + offsetTranslationStep
                    setTranslationOffset(offsetX, offsetY, value)
                }
            }
        }
    }
}
