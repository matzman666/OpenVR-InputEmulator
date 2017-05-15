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
            Layout.bottomMargin: 64

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
                    "Linear Approximation (Experimental)"
                ]
                onCurrentIndexChanged: {
                    if (setupFinished) {
                        DeviceManipulationTabController.setMotionCompensationVelAccMode(currentIndex)
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
            setupFinished = true
        }

        Connections {
            target: DeviceManipulationTabController
            onMotionCompensationVelAccModeChanged: {
                deviceSelectionComboBox.currentIndex = mode
            }
        }

    }

}
