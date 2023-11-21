import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0
import components.images 1.0

import onboard 1.0

StackViewPage {
    id: root

    property alias changeEmailLinkText: changeEmailLinkTextItem

    ColumnLayout {
        id: layout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: contentSpacing

        RichText {
            id: title

            rawText: OnboardingStrings.confirmEmailTitle
            wrapMode: Text.WordWrap
            color: Styles.textPrimary
            font {
                pixelSize: Text.Size.Large
                bold: true
            }
            Layout.fillWidth: true
        }

        Text {
            id: bodyText

            Layout.preferredWidth: layout.width
            text: OnboardingStrings.confirmEmailBodyText
            font.pixelSize: Text.Size.Medium
        }

        RichText {
            id: bodyText2

            rawText: OnboardingStrings.confirmEmailBodyText2
            font.pixelSize: Text.Size.Medium
            url: Links.contact
            manageMouse: true
            Layout.preferredWidth: layout.width
        }

        RowLayout {
            id: mailLayout

            spacing: 9
            Layout.preferredWidth: layout.width

            SvgImage {
                id: mailImage

                source: Images.mail
                sourceSize: Qt.size(24, 24)
                color: Styles.textPrimary
                Layout.alignment: Qt.AlignTop
            }

            Text {
                id: email

                text: loginControllerAccess.email
                wrapMode: Text.Wrap
                lineHeightMode: Text.FixedHeight
                lineHeight: 20
                font {
                    pixelSize: Text.Size.Medium
                    bold: true
                }
                Layout.preferredWidth: parent.width - mailImage.width - parent.spacing
                Layout.topMargin: 3
            }
        }

        RichText {
            id: changeEmailLinkTextItem

            font.pixelSize: Text.Size.Medium
            rawText: OnboardingStrings.confirmEmailChangeText
            wrapMode: Text.WordWrap
            color: Styles.textPrimary
            manageMouse: true
            Layout.preferredWidth: layout.width
        }
    }
}
