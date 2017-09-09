import QtQuick 2.9;
import QtQuick.Controls 2.2;
import QtQuick.Layouts 1.3;
import matzman666.inputemulator 1.0

RowLayout {

    property string buttonName: "<Button_Name>"
    property string buttonStatus: "<Button_Status>"
    property int deviceIndex: -1
    property int buttonId: -1

    MyText {
        Layout.preferredWidth: 270
        text: buttonName
    }
    MyText {
        id: buttonStatusText
        Layout.preferredWidth: 650
        text: buttonStatus
    }
    MyPushButton {
        Layout.preferredWidth: 150
        text: "Configure"
        onClicked: {
            DeviceManipulationTabController.startConfigureDigitalInputRemapping(deviceIndex, buttonId)
            deviceDigitalInputRemappingPage.setDeviceIndex(deviceIndex, buttonId)
            MyResources.playFocusChangedSound()
            var res = mainView.push(deviceDigitalInputRemappingPage)
        }
    }

    Connections {
        target: DeviceManipulationTabController
        onConfigureDigitalInputRemappingFinished: {
            var buttonStatus = DeviceManipulationTabController.getDigitalButtonStatus(deviceIndex, buttonId);
            buttonStatusText.text = buttonStatus
        }
    }
}
