import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import matzman666.inputemulator 1.0

MyStackViewPage {
    id: deviceInputRemappingPage

    headerText: "Input Remapping Settings "

    property int deviceIndex: -1

    property var objects: []

    property var _digitalButtonComponent: Qt.createComponent("DeviceDigitalButtonEntry.qml")
    property var _analogAxisComponent: Qt.createComponent("DeviceAnalogAxisEntry.qml")

    function createDigitalButton(i, buttonId) {
        var buttonName = DeviceManipulationTabController.getDigitalButtonName(deviceIndex, buttonId);
        var buttonStatus = DeviceManipulationTabController.getDigitalButtonStatus(deviceIndex, buttonId);
        var obj = _digitalButtonComponent.createObject(digitalButtonContainer, {
            "buttonName": buttonName,
            "buttonStatus": buttonStatus,
            "deviceIndex": deviceIndex,
            "buttonId": buttonId
        });
        if (obj == null) {
            console.log("Error creating object");
        }
        objects.push(obj)
    }

    function createAnalogAxis(i, axisId) {
        var axisName = DeviceManipulationTabController.getAnalogAxisName(deviceIndex, axisId);
        var axisStatus = DeviceManipulationTabController.getAnalogAxisStatus(deviceIndex, axisId);
        var obj = _analogAxisComponent.createObject(analogAxisContainer, {
            "axisName": axisName,
            "axisStatus": axisStatus,
            "deviceIndex": deviceIndex,
            "axisId": axisId
        });
        if (obj == null) {
            console.log("Error creating object");
        }
        objects.push(obj)
    }

    function deleteObjects() {
        for (var i = 0; i < objects.length; i++) {
            objects[i].destroy()
        }
        objects = []
    }

    function setDeviceIndex(index) {
        deviceIndex = index
        deleteObjects()
        var buttonCount = DeviceManipulationTabController.getDigitalButtonCount(index);
        if (buttonCount > 0) {
            digitalInputHeader.visible = true
            for (var i = 0; i < buttonCount; i++) {
                var buttonId = DeviceManipulationTabController.getDigitalButtonId(index, i);
                createDigitalButton(i, buttonId)
            }
        } else {
            digitalInputHeader.visible = false
        }
        var axisCount = DeviceManipulationTabController.getAnalogAxisCount(index);
        if (axisCount > 0) {
            analogInputHeader.visible = true
            for (var i = 0; i < axisCount; i++) {
                var axisId = DeviceManipulationTabController.getAnalogAxisId(index, i);
                createAnalogAxis(i, axisId)
            }
        } else {
            analogInputHeader.visible = false
        }
        if (buttonCount == 0 && axisCount == 0) {
            noInputsLabel.visible = true
        } else {
            noInputsLabel.visible = false
        }
    }

    function updateRemappingStatus() {

    }

    content: ColumnLayout {
        spacing: 18

        MyText {
            id: noInputsLabel
            visible: false
            text: "No Inputs"
        }

        ScrollView {
            ScrollBar.vertical.policy: ScrollBar.AsNeeded
            ScrollBar.vertical.width: 25
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ColumnLayout {
                spacing: 24

                RowLayout {
                    id: digitalInputHeader
                    visible: false
                    Layout.topMargin: 32
                    Layout.leftMargin: 250
                    Rectangle {
                        Layout.rightMargin: 10
                        color: "#ffffff"
                        height: 1
                        width: 200
                    }
                    MyText {
                        text: "Digital Inputs"
                    }
                    Rectangle {
                        Layout.leftMargin: 10
                        color: "#ffffff"
                        height: 1
                        width: 200
                    }
                }

                ColumnLayout {
                    id: digitalButtonContainer
                    spacing: 18
                }

                RowLayout {
                    id: analogInputHeader
                    visible: false
                    Layout.topMargin: 32
                    Layout.leftMargin: 250
                    Rectangle {
                        Layout.rightMargin: 10
                        color: "#ffffff"
                        height: 1
                        width: 200
                    }
                    MyText {
                        text: "Analog Inputs"
                    }
                    Rectangle {
                        Layout.leftMargin: 10
                        color: "#ffffff"
                        height: 1
                        width: 200
                    }
                }

                ColumnLayout {
                    id: analogAxisContainer
                    spacing: 24
                }
            }
        }


        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Component.onCompleted: {
        }

    }

}
