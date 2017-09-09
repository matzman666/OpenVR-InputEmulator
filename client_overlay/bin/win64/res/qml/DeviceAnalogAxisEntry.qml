import QtQuick 2.9;
import QtQuick.Controls 2.2;
import QtQuick.Layouts 1.3;

RowLayout {

    property string axisName: "<Axis_Name>"
    property string axisStatus: "<Axis_Status>"

    MyText {
        Layout.preferredWidth: 270
        text: axisName
    }
    MyText {
        Layout.preferredWidth: 650
        text: axisStatus
    }
    MyPushButton {
        Layout.preferredWidth: 150
        text: "Configure"
    }
}
