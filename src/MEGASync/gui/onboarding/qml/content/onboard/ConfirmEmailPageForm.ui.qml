// System
import QtQuick 2.15
import QtQuick.Layouts 1.15

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

// Local
import Onboard 1.0

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

        MegaTexts.RichText {
            id: title

            rawText: OnboardingStrings.confirmEmailTitle
            wrapMode: Text.WordWrap
            color: Styles.textPrimary
            font {
                pixelSize: MegaTexts.Text.Size.Large
                bold: true
            }
            Layout.fillWidth: true
        }

        MegaTexts.Text {
            id: bodyText

            Layout.preferredWidth: layout.width
            text: OnboardingStrings.confirmEmailBodyText
            font.pixelSize: MegaTexts.Text.Size.Medium
        }

        MegaTexts.RichText {
            id: bodyText2

            rawText: OnboardingStrings.confirmEmailBodyText2
            font.pixelSize: MegaTexts.Text.Size.Medium
            url: Links.contact
            manageMouse: true
            Layout.preferredWidth: layout.width
        }

        RowLayout {
            id: mailLayout

            spacing: 9
            Layout.preferredWidth: layout.width

            MegaImages.SvgImage {
                id: mailImage

                source: Images.mail
                sourceSize: Qt.size(24, 24)
                color: Styles.textPrimary
                Layout.alignment: Qt.AlignTop
            }

            MegaTexts.Text {
                id: email

                text: loginControllerAccess.email
                wrapMode: Text.Wrap
                lineHeightMode: Text.FixedHeight
                lineHeight: 20
                font {
                    pixelSize: MegaTexts.Text.Size.Medium
                    bold: true
                }
                Layout.preferredWidth: parent.width - mailImage.width - parent.spacing
                Layout.topMargin: 3
            }
        }

        MegaTexts.RichText {
            id: changeEmailLinkTextItem

            font.pixelSize: MegaTexts.Text.Size.Medium
            rawText: OnboardingStrings.confirmEmailChangeText
            wrapMode: Text.WordWrap
            color: Styles.textPrimary
            manageMouse: true
            Layout.preferredWidth: layout.width
        }
    }
}
