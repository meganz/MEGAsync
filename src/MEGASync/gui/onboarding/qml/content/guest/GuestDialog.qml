// System
import QtQuick 2.12
import QtQuick.Window 2.12

// Local
import Guest 1.0

// C++
import GuestQmlDialog 1.0
import LoginController 1.0

GuestQmlDialog {
    id: guestWindow

    width: guestItem.width
    height: guestItem.height
    modality: Qt.NonModal
    flags: Qt.FramelessWindowHint
    color: "transparent"

    GuestItem {
        id: guestItem
    }

    onVisibleChanged:  {
        if(visible) {
            AccountStatusControllerAccess.whyAmIBlocked();
            if(LoginControllerAccess.state !== LoginController.SIGN_UP
                    && LoginControllerAccess.state !== LoginController.LOGGED_OUT)
            {
                LoginControllerAccess.guestWindowButtonClicked();
            }
        }
    }
}
