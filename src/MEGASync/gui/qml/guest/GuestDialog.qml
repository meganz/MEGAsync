import QtQuick 2.15

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
            accountStatusControllerAccess.whyAmIBlocked();
        }
    }

    onGuestActiveChanged: (active) => {
        if (!active) {
            fadeOut.start();
        }
    }

    PropertyAnimation {
        id: fadeOut;

        target: guestWindow;
        property: "opacity";
        to: 0;
        duration: 100

        onRunningChanged: {
            if (!running) {
                guestWindow.hide()
                guestWindow.opacity = 1;
            }
        }
    }
}
