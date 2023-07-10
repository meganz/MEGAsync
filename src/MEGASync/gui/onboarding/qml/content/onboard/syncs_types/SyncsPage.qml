// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0

Rectangle {

    property alias footerButtons: footerButtons

    color: Styles.surface1

    Footer {
        id: footerButtons

        anchors {
            bottom: parent.bottom
            right: parent.right
            left: parent.left
        }
    }
}
