// System
import QtQuick 2.12
import QtQuick.Window 2.12

// C++
import GuestQmlDialog 1.0
import LoginController 1.0

GuestQmlDialog {
    id: root

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
