pragma Singleton
import QtQuick 2.9
import QtMultimedia 5.6
import matzman666.inputemulator 1.0

QtObject {
	function playActivationSound() {
        OverlayController.playActivationSound()
	}
	function playFocusChangedSound() {
        OverlayController.playFocusChangedSound()
    }
}
