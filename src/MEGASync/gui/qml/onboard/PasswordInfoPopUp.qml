// System
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

// QML common
import common 1.0
import components.images 1.0

Popup {
    id: root

    property alias allChecked: content.allChecked
    property alias validPassword: content.validPassword

    property string password: ""

    focus: false
    closePolicy: Popup.NoAutoClose

    background: Item {
        width: content.width + tip.width
        height: content.height

        DropShadow {
            id: shadow

            anchors.fill: parent
            radius: 16.0
            samples: 25
            cached: true
            color: "#44000000"
            source: mainRow
        }
    }

    contentItem: Item {
        anchors.top: parent.top
        anchors.left: parent.left
        width: content.width + tip.width
        height: content.height

        Row {
            id: mainRow

            anchors.fill: parent

            PasswordInfoContent {
                id: content

                password: root.password
            }

            SvgImage {
                id: tip

                anchors.verticalCenter: parent.verticalCenter
                source: Images.tip
                sourceSize: Qt.size(12, 48)
            }
        }
    }

}
