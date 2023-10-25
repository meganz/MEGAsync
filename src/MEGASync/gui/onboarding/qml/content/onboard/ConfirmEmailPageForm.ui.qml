// System
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// QML common
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Components.Buttons 1.0 as MegaButtons
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

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
        spacing: 24

        MegaTexts.RichText {
            Layout.fillWidth: true
            font.pixelSize: MegaTexts.Text.Size.Large
            font.bold: true
            rawText: OnboardingStrings.confirmEmailTitle
            wrapMode: Text.WordWrap
            color: Styles.textPrimary
        }

        MegaTexts.Text {
            Layout.preferredWidth: layout.width
            text: OnboardingStrings.confirmEmailBodyText
            font.pixelSize: MegaTexts.Text.Size.Medium
        }

        MegaTexts.RichText {
            Layout.preferredWidth: layout.width
            rawText: OnboardingStrings.confirmEmailBodyText2
            font.pixelSize: MegaTexts.Text.Size.Medium
            url: Links.contact
            manageMouse: true;
        }

        RowLayout {
            spacing: 9
            Layout.preferredWidth: layout.width

            MegaImages.SvgImage {
                id: mailImage

                source: Images.mail
                sourceSize: Qt.size(24, 24)
                Layout.alignment: Qt.AlignTop
                color: Styles.textPrimary
            }

            MegaTexts.Text {
                id: email

                text: loginControllerAccess.email
                Layout.preferredWidth: parent.width - mailImage.width - parent.spacing
                Layout.topMargin: 3
                font.pixelSize: MegaTexts.Text.Size.Medium
                font.bold: true
                wrapMode: Text.Wrap
                lineHeightMode: Text.FixedHeight
                lineHeight: 20
            }
        }

        MegaTexts.RichText {
            id: changeEmailLinkTextItem

            Layout.preferredWidth: layout.width
            font.pixelSize: MegaTexts.Text.Size.Medium
            rawText: OnboardingStrings.confirmEmailChangeText
            wrapMode: Text.WordWrap
            color: Styles.textPrimary
            manageMouse: true
        }
    }
}
