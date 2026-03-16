import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.progressBars 1.0

import ServiceUrls 1.0

Item {
    id: root

    property alias leftButton: leftButton
    property alias rightButton: rightButton

    readonly property double imageTopMargin: 78
    readonly property int spacing: 48
    readonly property int horizontalMargin: 24
    readonly property int topMargin: 48
    readonly property int mainIconSize: 96
    readonly property int bottomInfoIconSize: 16
    readonly property int descriptionLineHeight: 18
    readonly property int bottomInfoSpacing: 4
    readonly property int bottomRowSpacing: 12
    readonly property int colSpacing: 12
    readonly property int buttonRowSpacing: 4
    readonly property int titleHeight: 24
    readonly property int bottomInfoLinkHeight: 26

    Image {
        id: image

        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
            topMargin: imageTopMargin
        }

        height: mainIconSize
        width: mainIconSize
        source: Images.warningGuest
        sourceSize.width: width
        sourceSize.height: height
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    Column {
        id: mainColumn

        anchors {
            top: image.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: horizontalMargin
            rightMargin: horizontalMargin
            topMargin: topMargin
        }
        spacing: root.spacing

        Column {
            id: textColumn

            anchors {
                left: parent.left
                right: parent.right
            }

            spacing: colSpacing

            Texts.Text {
                id: title

                anchors {
                    left: parent.left
                    right: parent.right
                }
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font {
                    pixelSize: Texts.Text.Size.MEDIUM_LARGE
                    weight: Font.DemiBold
                }
                lineHeight: titleHeight
                lineHeightMode: Text.FixedHeight
                text: GuestStrings.accountTempLocked
                visible: true
            }

            Texts.RichText {
                id: description

                anchors {
                    left: parent.left
                    right: parent.right
                }
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: ColorTheme.textPrimary
                manageMouse: true
                font.pixelSize: Texts.Text.Size.NORMAL
                lineHeight: descriptionLineHeight
                lineHeightMode: Text.FixedHeight
                rawText: GuestStrings.accountTempLockedEmail
                visible: true
            }
        }

        Column {
            id: buttonsCol

            spacing: bottomRowSpacing
            anchors {
                left: parent.left
                right: parent.right
            }

            Row {
                id: buttonsRow

                anchors.horizontalCenter: parent.horizontalCenter
                spacing: buttonRowSpacing

                OutlineButton {
                    id: leftButton

                    text: GuestStrings.resendEmail
                    onClicked: {
                        guestContentAccess.onVerifyEmailClicked();
                    }
                }

                PrimaryButton {
                    id: rightButton

                    text: GuestStrings.logOut
                    onClicked: {
                        guestContentAccess.onLogoutClicked();
                    }
                }
            }

            Row {
                id: rowBottomInfoLink

                spacing: bottomInfoSpacing
                anchors.horizontalCenter: parent.horizontalCenter
                height: bottomInfoLinkHeight

                LinkButton {
                    text: GuestStrings.whyISeeThis
                    icons {
                        source: Images.externalLink
                        position: Icon.Position.RIGHT
                    }
                    buttonCursorShape: Qt.PointingHandCursor
                    sizes.textFontSize: Texts.Text.Size.NORMAL
                    url: serviceUrlsAccess.getCredentialStuffingHelpUrl()
                }
            }
        }
    }
}
