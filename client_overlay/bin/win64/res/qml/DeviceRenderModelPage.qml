import QtQuick 2.9
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import matzman666.inputemulator 1.0

MyStackViewPage {
    id: deviceRenderModelPage

    headerText: "Render Model Settings"

    property int deviceIndex: -1

    function setDeviceIndex(index) {
        deviceIndex = index
        var renderModelCount = DeviceManipulationTabController.getRenderModelCount()
        var renderModels = ["<Disabled>"]
        for (var i = 0; i < renderModelCount; i++) {
            renderModels.push(DeviceManipulationTabController.getRenderModelName(i))
            deviceRenderModelComboBox.model = renderModels
        }
    }

    content: ColumnLayout {
        spacing: 18

        RowLayout {
            spacing: 18

            MyText {
                text: "Render Model:"
            }

            MyComboBox {
                id: deviceRenderModelComboBox
                Layout.maximumWidth: 910
                Layout.minimumWidth: 910
                Layout.preferredWidth: 910
                Layout.fillWidth: true
                model: ["<Disabled>"]
                onCurrentIndexChanged: {
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        RowLayout {
            spacing: 18

            Item {
                Layout.fillWidth: true
            }

            MyPushButton {
                id: deviceRenderModelButton
                Layout.preferredWidth: 194
                Layout.rightMargin: 32
                text: "Save"
                onClicked: {
                    DeviceManipulationTabController.setDeviceRenderModel(deviceIndex, deviceRenderModelComboBox.currentIndex)
                }
            }
        }


        Component.onCompleted: {
        }

        Connections {
            target: DeviceManipulationTabController
        }

    }

}
