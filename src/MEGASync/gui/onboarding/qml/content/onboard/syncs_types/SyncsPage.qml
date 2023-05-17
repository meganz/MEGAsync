// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0

Rectangle {

    property alias footerButtons: footerButtons

    readonly property int contentMargin: 32


    Footer {
        id: footerButtons

        anchors {
            bottom: parent.bottom
            right: parent.right
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
