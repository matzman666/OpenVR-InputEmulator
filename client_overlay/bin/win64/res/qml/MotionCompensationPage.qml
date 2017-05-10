import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import matzman666.inputemulator 1.0

MyStackViewPage {
    id: motionCompensationPage

    headerText: "Motion Compensation Settings"

    content: ColumnLayout {
        spacing: 18

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
