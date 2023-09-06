// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

// QML common
import Common 1.0
import Components.Images 1.0 as MegaImages

Popup {
    id: passwordPopup

    property alias allChecked: content.allChecked
    property alias validPassword: content.validPassword

    property string password: ""

    focus: false
    closePolicy: Popup.NoAutoClose

    background: Rectangle {
        color: "transparent"

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        width: content.width + tip.width
        height: content.height

        DropShadow {
            anchors.fill: parent
            radius: 16.0
            samples: 25
            cached: true
            color: "#44000000"
            source: theRow
        }
    }

    contentItem: Rectangle {
        color: "transparent"

        anchors.top: parent.top
        anchors.left: parent.left
        width: content.width + tip.width
        height: content.height

        Row {
            id: theRow

            anchors.fill: parent

            PasswordInfoContent {
                id: content

                password: passwordPopup.password
            }

            MegaImages.SvgImage {
                id: tip

                anchors.verticalCenter: parent.verticalCenter
                source: Images.tip
                sourceSize: Qt.size(12, 48)
            }
        }
    }

}
