import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

import onboard 1.0

StackViewPage {
    id: root

    property alias changeEmailLinkText: changeEmailLinkTextItem
    property alias bodyText2 : bodyText2Item

    ColumnLayout {
        id: layout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: contentSpacing

        Texts.RichText {
            id: title

            rawText: OnboardingStrings.confirmEmailTitle
            wrapMode: Text.WordWrap
            color: Styles.textPrimary
            font {
                pixelSize: Texts.Text.Size.Large
                bold: true
            }
            Layout.fillWidth: true
        }

        Texts.Text {
            id: bodyText

            Layout.preferredWidth: layout.width
            text: OnboardingStrings.confirmEmailBodyText
            font.pixelSize: Texts.Text.Size.Medium
        }

        Texts.RichText {
            id: bodyText2Item

            rawText: OnboardingStrings.confirmEmailBodyText2
            font.pixelSize: Texts.Text.Size.Medium
            url: Links.contact
            manageMouse: true
            Layout.preferredWidth: layout.width
            KeyNavigation.tab: changeEmailLinkTextItem
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

            Texts.Text {
                id: email

                text: loginControllerAccess.email
                wrapMode: Text.Wrap
                lineHeightMode: Text.FixedHeight
                lineHeight: 20
                font {
                    pixelSize: Texts.Text.Size.Medium
                    bold: true
                }
                Layout.preferredWidth: parent.width - mailImage.width - parent.spacing
                Layout.topMargin: 3
            }
        }

        Texts.RichText {
            id: changeEmailLinkTextItem

            font.pixelSize: Texts.Text.Size.Medium
            rawText: OnboardingStrings.confirmEmailChangeText
            wrapMode: Text.WordWrap
            color: Styles.textPrimary
            manageMouse: true
            Layout.preferredWidth: layout.width
            KeyNavigation.tab: bodyText2Item
        }
    }
}
