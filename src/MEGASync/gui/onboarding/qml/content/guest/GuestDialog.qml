// System
import QtQuick 2.15
import QtQuick.Window 2.15

// Local
import Guest 1.0

// C++
import GuestQmlDialog 1.0
import LoginController 1.0

GuestQmlDialog {
    id: guestWindow

    width: guestItem.width
    height: guestItem.height
    color: "transparent"

    GuestItem {
        id: guestItem
    }

    onVisibleChanged:  {
        if(visible) {
            AccountStatusControllerAccess.whyAmIBlocked();
        }
    }
}
