// System
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

// QML common
import Common 1.0

Rectangle {

    property alias footerButtons: footerButtons

    color: Styles.surface1

    Footer {
        id: footerButtons
    }
}
