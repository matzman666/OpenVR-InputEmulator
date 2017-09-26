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
    MyPushButton {
        id: configButton
        Layout.preferredWidth: 840
        text: buttonStatus
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
            buttonStatus = DeviceManipulationTabController.getDigitalButtonStatus(deviceIndex, buttonId);
            configButton.text = buttonStatus
        }
    }
}
