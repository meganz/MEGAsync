// System
import QtQuick 2.15

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

// Local
import Onboard 1.0

Rectangle {

    property alias conditionUpperLowerCase: conditionUpperLowerCase
    property alias conditionNumberSpecialCharacter: conditionNumberSpecialCharacter

    property string password: ""

    readonly property int contentMargin: 24
    readonly property int iconWidth: 24
    readonly property int lineHeight: 2
    readonly property int conditionSpacing: 12

    width: 320
    height: 180
    color: Styles.pageBackground
    radius: 8

    Column {
        anchors.fill: parent
        anchors.margins: contentMargin
        spacing: contentMargin

        MegaTexts.Text {
            id: strengthTitle

            width: parent.width
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            text: OnboardingStrings.passwordAtleast8Chars
            font.strikeout: password.length >= 8
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

        }
    }

}
