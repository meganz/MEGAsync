// System
import QtQuick 2.12
import QtQuick.Window 2.12

// QML common
import com.qmldialog 1.0

QmlDialog {

    function realocate() {
        var screenWidth = Screen.width;
        var screenHeight = Screen.height;
        console.debug("mock GuestWindow::realocate() : " + "resolution -> " + screenWidth + "x" + screenHeight);

        // Change to emulate a different OS
        var os = Qt.platform.os;
        console.debug("mock GuestWindow::realocate() : " + "emulate -> " + os);
        switch(os) {
            case "windows":
                x = screenWidth - width;
                // Emulated taskbar size = 75
                y = screenHeight - height - 75;
                break;
            case "osx":
                x = screenWidth - width;
                // Emulated taskbar size = 30
                y = 30;
                break;
            case "linux":
                x = screenWidth - width;
                // Emulated taskbar size = 30
                y = 30;
                break;
        }

        console.debug("mock GuestWindow::realocate() : " + "x -> " + x + " : y -> " + y);
    }

}
