import QtQuick 2.9;
import QtQuick.Controls 2.2;
import QtQuick.Layouts 1.3;
import matzman666.inputemulator 1.0

RowLayout {

    property string axisName: "<Axis_Name>"
    property string axisStatus: "<Axis_Status>"
    property int deviceIndex: -1
    property int axisId: -1

    MyText {
        Layout.preferredWidth: 270
        text: axisName
    }
    MyPushButton {
        id: configButton
        Layout.preferredWidth: 840
        text: axisStatus
        onClicked: {
            DeviceManipulationTabController.startConfigureAnalogInputRemapping(deviceIndex, axisId)
            deviceAnalogInputRemappingPage.setDeviceIndex(deviceIndex, axisId)
            MyResources.playFocusChangedSound()
            var res = mainView.push(deviceAnalogInputRemappingPage)
        }
    }

    Connections {
        target: DeviceManipulationTabController
        onConfigureAnalogInputRemappingFinished: {
            axisStatus = DeviceManipulationTabController.getAnalogAxisStatus(deviceIndex, axisId);
            configButton.text = axisStatus
        }
    }
}
