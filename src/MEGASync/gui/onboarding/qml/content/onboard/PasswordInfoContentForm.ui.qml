// System
import QtQuick 2.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

// Local
import Onboard 1.0

Rectangle {

    property alias strengthTitle: strengthTitle
    property alias strenghtIcon: strenghtIcon
    property alias upperLowerCaseChecked: conditionUpperLowerCase.checked
    property alias numberSpecialCharacterChecked: conditionNumberSpecialCharacter.checked
    property alias longerChecked: conditionLonger.checked

    readonly property int contentMargin: 24
    readonly property int iconWidth: 24
    readonly property int lineHeight: 2
    readonly property int conditionSpacing: 12

    width: 320
    height: 225
    color: Styles.pageBackground
    radius: 8

    Column {
        anchors.fill: parent
        anchors.margins: contentMargin
        spacing: contentMargin

        Row {
            width: parent.width

            MegaTexts.Text {
                id: strengthTitle

                width: parent.width - strenghtIcon.width
                height: strenghtIcon.height
                font.pixelSize: MegaTexts.Text.Size.MediumLarge
                font.bold: true
                verticalAlignment: Text.AlignVCenter
                text: OnboardingStrings.passwordAtleast8Chars
            }

            MegaImages.SvgImage {
                id: strenghtIcon

                source: Images.passwordVeryWeak
                sourceSize: Qt.size(iconWidth, iconWidth)
                color: Styles.iconSecondary
                visible: false
            }
        }

        Rectangle {
            width: parent.width
            height: lineHeight
            radius: height
            color: Styles.divider
        }

        Column {
            width: parent.width
            spacing: conditionSpacing

            MegaTexts.SecondaryText {
                width: parent.width
                text: OnboardingStrings.itsBetterToHave;
                font.weight: Font.DemiBold
            }

            PasswordConditionItem {
                id: conditionUpperLowerCase

                text: OnboardingStrings.upperAndLowerCase;
            }

            PasswordConditionItem {
                id: conditionNumberSpecialCharacter

                text: OnboardingStrings.numberOrSpecialChar;
            }

            PasswordConditionItem {
                id: conditionLonger

                text: OnboardingStrings.longerPassword;
            }

        }
    }

}
