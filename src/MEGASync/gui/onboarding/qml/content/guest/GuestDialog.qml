// System
import QtQuick 2.15
import QtQuick.Window 2.15

// C++
import GuestQmlDialog 1.0
import LoginController 1.0

GuestQmlDialog {
    id: guestWindow

    width: guestFlow.width
    height: guestFlow.height
    color: "transparent"

    GuestFlow {
        id: guestFlow
    }

    onVisibleChanged:  {
        if(visible) {
            AccountStatusControllerAccess.whyAmIBlocked();
        }
    }
}
