import QtQuick 2.12
import QtQuick.Window 2.12

import Guest 1.0
import GuestQmlDialog 1.0

GuestQmlDialog {
    id: guestWindow

    width: guestContent.width
    height: guestContent.height
    modality: Qt.NonModal
    flags: Qt.FramelessWindowHint
    color: "transparent"

    GuestContent {
        id: guestContent
    }

    Component.onCompleted: {
        guestWindow.realocate();
    }
}
