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
    MyText {
        id: axisStatusText
        Layout.preferredWidth: 650
        text: axisStatus
    }
    MyPushButton {
        Layout.preferredWidth: 150
        text: "Configure"
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
            var axisStatus = DeviceManipulationTabController.getAnalogAxisName(deviceIndex, axisId);
            axisStatusText.text = axisStatus
        }
    }
}
