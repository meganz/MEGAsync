// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtGraphicalEffects 1.0

// QML common
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Components.Buttons 1.0 as MegaButtons
import Common 1.0

// Local
import Onboard 1.0
import Guest 1.0

Rectangle {

    property alias loginButton: loginButton
    property alias signUpButton: signUpButton
    property alias menuButton: menuButton
    property alias menu: menu
    property alias aboutMenuItem: aboutMenuItem
    property alias preferencesItem: preferencesItem
    property alias exitItem: exitItem

    width: 400
    height: 560
    radius: 10
    color: Styles.surface1

    MegaImages.SvgImage {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 16
        anchors.leftMargin: 16
        source: Images.mega
        sourceSize: Qt.size(24, 24)
    }

    MegaButtons.IconButton {
        id: menuButton

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 9
        anchors.rightMargin: 9
        icons.source: Images.menu
    }

    Qml.Menu {
        id: menu

        x: menuButton.x + menuButton.width - width
        y: menuButton.y + menuButton.width + 4
        dim: true
        modal: true

        Qml.Overlay.modal: Rectangle {
            color: "transparent"
        }

        background: Rectangle {
            implicitWidth: 200
            implicitHeight: 120
            color: Styles.pageBackground
            radius: 12
            layer.enabled: true
            layer.effect: DropShadow {
                transparentBorder: true
                horizontalOffset: 4
                verticalOffset: 8
                radius: 8.0
                samples: 16
                cached: true
                color: "#10000000"
            }
        }

        MenuItem {
            id: aboutMenuItem

            text: GuestStrings.menuAboutMEGA
            icon.source: Images.megaOutline
            position: MenuItem.Position.First
        }

        MenuItem {
            id: preferencesItem

            text: GuestStrings.menuPreferences
            icon.source: Images.settings
        }

        MenuItem {
            id: exitItem

            text: GuestStrings.menuExit
            icon.source: Images.exit
            position: MenuItem.Position.Last
        }
    }

    Image {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 72
        source: Images.guest
    }

    Column {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 48
        spacing: 24

        Column {
            spacing: 12
            width: 352

            MegaTexts.Text {
                anchors.left: parent.left
                anchors.right: parent.right
                text: GuestStrings.title
                font.pixelSize: MegaTexts.Text.Size.MediumLarge
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            MegaTexts.Text {
                anchors.left: parent.left
                anchors.right: parent.right
                text: GuestStrings.description
                color: Styles.textSecondary
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

        }

        Row {
            spacing: 8
            anchors.horizontalCenter: parent.horizontalCenter

            MegaButtons.OutlineButton {
                id: signUpButton

                text: OnboardingStrings.signUp
            }

            MegaButtons.PrimaryButton {
                id: loginButton

                text: OnboardingStrings.login
            }
        }
    }
}
