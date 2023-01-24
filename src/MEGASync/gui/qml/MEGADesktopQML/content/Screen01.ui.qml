/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick 2.12
import QtQuick.Controls 2.12
import Styles 1.0

Rectangle {
    width: Styles.width
    height: Styles.height
    color: Styles.backgroundColor

    Text {
        text: qsTr("Hello MEGADesktopQML")
        anchors.centerIn: parent
        font.family: Styles.font.family
    }

    Dial {
        id: dial
        x: 261
        y: 117
        width: 100
        height: 103
    }

    BusyIndicator {
        id: busyIndicator
        x: 290
        y: 297
    }
}
