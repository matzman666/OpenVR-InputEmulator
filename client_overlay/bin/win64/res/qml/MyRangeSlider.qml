import QtQuick 2.9
import QtQuick.Controls 2.2
import "." // QTBUG-34418, singletons require explicit import to load qmldir file


RangeSlider {
    id: rangeSlider
    snapMode: Slider.SnapAlways
    wheelEnabled: true
    hoverEnabled: true

    background: Rectangle {
        x: rangeSlider.leftPadding
        y: rangeSlider.topPadding + rangeSlider.availableHeight / 2 - height / 2
        implicitWidth: 200
        implicitHeight: 8
        width: rangeSlider.availableWidth
        height: implicitHeight
        color: "#cccccc"
        radius: 8

        Rectangle {
            x: rangeSlider.first.visualPosition * parent.width
            width: rangeSlider.second.visualPosition * parent.width - x
            height: parent.height
			color: "#2c435d"
            radius: 8
        }
    }

    first.handle: Rectangle {
        x: rangeSlider.leftPadding + rangeSlider.first.visualPosition * (rangeSlider.availableWidth - width)
        y: rangeSlider.topPadding + rangeSlider.availableHeight / 2 - height / 2
        implicitWidth: 20
        implicitHeight: 40
        radius: 2
        color: rangeSlider.first.pressed ? "#ffffff" : "#eeeeee"
        border.color: "#bdbebf"
    }

    second.handle: Rectangle {
        x: rangeSlider.leftPadding + rangeSlider.second.visualPosition * (rangeSlider.availableWidth - width)
        y: rangeSlider.topPadding + rangeSlider.availableHeight / 2 - height / 2
        implicitWidth: 20
        implicitHeight: 40
        radius: 4
        color: rangeSlider.second.pressed ? "#ffffff" : "#eeeeee"
        border.color: "#bdbebf"
    }

    onHoveredChanged: {
        if (hovered) {
            forceActiveFocus()
        } else {
            focus = false
        }
    }

    first.onValueChanged: {
        if (activeFocus) {
            MyResources.playActivationSound()
        }
    }

    second.onValueChanged: {
        if (activeFocus) {
            MyResources.playActivationSound()
        }
    }
}
