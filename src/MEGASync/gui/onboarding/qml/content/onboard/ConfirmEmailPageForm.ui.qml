// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

Rectangle {
    id: root

    property alias email: email.text
    property alias changeEmailLinkText: changeEmailLinkText

    readonly property int contentMargin: 48

    color: Styles.surface1

    ColumnLayout {
        id: layout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            rightMargin: contentMargin
            leftMargin: contentMargin
            topMargin: contentMargin
        }
        spacing: 24

        MegaTexts.RichText {
            id: title

            Layout.fillWidth: true
            font.pixelSize: MegaTexts.Text.Size.Large
            text: OnboardingStrings.confirmEmailTitle
            wrapMode: Text.WordWrap
            color: Styles.textPrimary
        }

        MegaTexts.Text {
            Layout.preferredWidth: layout.width
            text: OnboardingStrings.confirmEmailBodyText
            font.pixelSize: MegaTexts.Text.Size.Medium
        }

        RowLayout {
            spacing: 9

            MegaImages.SvgImage {
                source: Images.mail
                sourceSize: Qt.size(24, 24)
                Layout.alignment: Qt.AlignVCenter
                color: Styles.textPrimary
            }

            MegaTexts.Text {
                id: email

                Layout.alignment: Qt.AlignVCenter
                font.pixelSize: MegaTexts.Text.Size.Medium
                font.bold: true
            }
        }

        MegaTexts.RichText {
            id: changeEmailLinkText

            Layout.preferredWidth: layout.width
            font.pixelSize: MegaTexts.Text.Size.Medium
            text: OnboardingStrings.confirmEmailChangeText
            wrapMode: Text.WordWrap
            color: Styles.textPrimary
            manageMouse: true
        }
    }
}
