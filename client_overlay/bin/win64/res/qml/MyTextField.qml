import QtQuick 2.9
import QtQuick.Controls 2.0
import matzman666.inputemulator 1.0

TextField {
    property int _keyBoardUID: -1
    property string savedText: ""
    id: myTextField
    color: "#cccccc"
    text: ""
    font.pointSize: 20
    background: Button {
        hoverEnabled: true
        background: Rectangle {
            color: parent.hovered ? "#2c435d" : "#1b2939"
            border.color: "#cccccc"
            border.width: 2
        }
        onClicked: {
            myTextField.forceActiveFocus()
        }
    }
    onActiveFocusChanged: {
        if (activeFocus) {
            if (!OverlayController.desktopMode) {
                if (_keyBoardUID < 0) {
                    _keyBoardUID = OverlayController.getNewUniqueNumber()
                }
                OverlayController.showKeyboard(text, _keyBoardUID)
            } else {
                savedText = text
            }
        }
    }
    onEditingFinished: {
        if (OverlayController.desktopMode && savedText !== text) {
            myTextField.onInputEvent(text)
        }
    }
    function onInputEvent(input) {
        text = input
	}
    Connections {
        target: OverlayController
        onKeyBoardInputSignal: {
            if (_keyBoardUID >= 0 && userValue == _keyBoardUID) {
                if (myTextField.text !== input) {
                    myTextField.onInputEvent(input)
                }
            }
        }
    }
}
