import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

Rectangle {
    id: root

    readonly property int contentMargin: 24
    readonly property int iconWidth: 24
    readonly property int lineHeight: 2
    readonly property int conditionSpacing: 12

    property alias conditionUpperLowerCase: conditionUpperLowerCaseItem
    property alias conditionNumberSpecialCharacter: conditionNumberSpecialCharacterItem

    property string password: ""

    width: 320
    height: 180
    color: Styles.pageBackground
    radius: 8

    Column {
        id: mainColumn

        anchors {
            fill: parent
            margins: contentMargin
        }
        spacing: contentMargin

        Texts.Text {
            id: strengthTitle

            width: parent.width
            verticalAlignment: Text.AlignVCenter
            text: OnboardingStrings.passwordAtleast8Chars
            font {
                strikeout: password.length >= 8
                bold: true
            }
        }

        Rectangle {
            id: dividerLine

            width: parent.width
            height: lineHeight
            radius: height
            color: Styles.divider
        }

        Column {
            id: bottomColumn

            width: parent.width
            spacing: conditionSpacing

            Texts.SecondaryText {
                id: upperLine

                width: parent.width
                text: OnboardingStrings.itsBetterToHave;
                font.weight: Font.DemiBold
            }

            PasswordConditionItem {
                id: conditionUpperLowerCaseItem

                text: OnboardingStrings.upperAndLowerCase;
            }

            PasswordConditionItem {
                id: conditionNumberSpecialCharacterItem

                text: OnboardingStrings.numberOrSpecialChar;
            }

        }
    }

}
