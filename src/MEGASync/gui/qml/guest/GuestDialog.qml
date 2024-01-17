import QtQuick 2.15

import GuestQmlDialog 1.0
import LoginController 1.0

GuestQmlDialog {
    id: window

    width: guestFlow.width
    height: guestFlow.height
    color: "transparent"

    GuestFlow {
        id: guestFlow
    }

    onVisibleChanged:  {
        if(visible) {
            accountStatusControllerAccess.whyAmIBlocked();
        }
    }

    onGuestActiveChanged: (active) => {
        if (!active) {
            fadeOut.start();
        }
    }

    PropertyAnimation {
        id: fadeOut

        target: window
        property: "opacity"
        to: 0
        duration: 100

        onRunningChanged: {
            if (!running) {
                window.hide();
                window.opacity = 1;
            }
        }
    }
}
