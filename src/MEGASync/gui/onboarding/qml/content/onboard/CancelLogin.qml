import QtQuick.Window 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0

// Local
import Onboard 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Buttons 1.0 as MegaButtons

Window {
    id: dialog

    signal accepted;

    minimumWidth: 560
    minimumHeight: 284
    width: 560
    height: 284
    color: Styles.surface1

    RowLayout{
        id: rowLayout

        anchors {
            left: parent.left
            top: parent.top
            leftMargin: 48
            topMargin: 48
        }
        width: 464
        height: 100
        spacing: 24

        Image {
            source: Images.warning
        }

        ColumnLayout {
            spacing: 8

            MegaTexts.Text {
                font.pixelSize: MegaTexts.Text.Size.MediumLarge
                font.weight: Font.DemiBold
                text: OnboardingStrings.cancelLoginTitle
                lineHeightMode: Text.FixedHeight
                lineHeight: 24
            }

            MegaTexts.Text {
                font.pixelSize: MegaTexts.Text.Size.Normal
                text: OnboardingStrings.cancelLoginBodyText
                Layout.preferredWidth: 340
                lineHeightMode: Text.FixedHeight
                lineHeight: 18
            }

        }
    }

    RowLayout{

        anchors {
            right: parent.right
            bottom: parent.bottom
            top: rowLayout.bottom
            rightMargin: 48
            bottomMargin: 48
            topMargin: 24
        }
        spacing: 8

        MegaButtons.OutlineButton {
            text: OnboardingStrings.cancelLoginSecondaryButton
            onClicked: {
                dialog.close();
            }
        }

        MegaButtons.PrimaryButton {
            text: OnboardingStrings.cancelLoginPrimaryButton
            onClicked: {
                dialog.accepted();
            }
        }
    }
}
