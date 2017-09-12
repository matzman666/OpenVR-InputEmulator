import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.0
import "." // QTBUG-34418, singletons require explicit import to load qmldir file


Rectangle {
    id: root
    color: "#1b2939"
    width: 1200
    height: 800

    property DeviceManipulationPage devicePage: DeviceManipulationPage {
        stackView: mainView
    }

    property DeviceOffsetsPage deviceOffsetsPage:  DeviceOffsetsPage {
        stackView: mainView
    }

    property MotionCompensationPage motionCompensationPage:  MotionCompensationPage {
        stackView: mainView
    }

    property DeviceRenderModelPage deviceRenderModelPage:  DeviceRenderModelPage {
        stackView: mainView
    }

    property DeviceInputRemappingPage deviceInputRemappingPage:  DeviceInputRemappingPage {
        stackView: mainView
    }

    property DeviceDigitalInputRemappingPage deviceDigitalInputRemappingPage:  DeviceDigitalInputRemappingPage {
        stackView: mainView
    }

    property DeviceDigitalBindingPage deviceDigitalBindingPage:  DeviceDigitalBindingPage {
        stackView: mainView
    }

    property DeviceAnalogInputRemappingPage deviceAnalogInputRemappingPage:  DeviceAnalogInputRemappingPage {
        stackView: mainView
    }


    StackView {
        id: mainView
        anchors.fill: parent

		pushEnter: Transition {
			PropertyAnimation {
				property: "x"
				from: mainView.width
				to: 0
				duration: 200
			}
		}
		pushExit: Transition {
			PropertyAnimation {
				property: "x"
				from: 0
				to: -mainView.width
				duration: 200
			}
		}
		popEnter: Transition {
			PropertyAnimation {
				property: "x"
				from: -mainView.width
				to: 0
				duration: 200
			}
		}
		popExit: Transition {
			PropertyAnimation {
				property: "x"
				from: 0
				to: mainView.width
				duration: 200
			}
		}

        initialItem: devicePage
    }
}
