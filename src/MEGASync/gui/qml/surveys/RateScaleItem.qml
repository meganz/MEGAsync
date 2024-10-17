import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0
import components.buttons 1.0

Item {
    id: root

    property int numRateItems: 5
    property int score: -1

    height: content.height

    Column {
        id: content

        width: parent.width

        readonly property real defaultLineHeight: 20

        Row {
            width: parent.width
            height: rateItem.height

            Repeater {
                id: rateRepeater

                model: root.numRateItems
                delegate: IconButton {
                    id: rateItem

                    property color colorChecked: checked
                                                 ? ColorTheme.supportWarning
                                                 : ColorTheme.iconDisabled

                    icons {
                        colorEnabled: colorChecked
                        colorDisabled: colorChecked
                        colorHovered: colorChecked
                        colorPressed: colorChecked
                        source: checked ? Images.starFilled : Images.starEmpty
                    }
                    colors.pressed: "transparent"
                    sizes.iconSize: Qt.size(34, 33)
                    onClicked: {
                        score = index;
                        for (var i = 0; i < root.numRateItems; i++) {
                            rateRepeater.itemAt(i).checked = i <= score;
                        }
                    }
                }
            }
        }

        Row {
            width: parent.width
            spacing: 0

            SecondaryText {
                width: parent.width / 2
                font.pixelSize: Text.Size.MEDIUM
                lineHeight: content.defaultLineHeight
                lineHeightMode: Text.FixedHeight
                horizontalAlignment: Text.AlignLeft
                text: OneQuestionSurveyStrings.poor
            }

            SecondaryText {
                width: parent.width / 2
                font.pixelSize: Text.Size.MEDIUM
                lineHeight: content.defaultLineHeight
                lineHeightMode: Text.FixedHeight
                horizontalAlignment: Text.AlignRight
                text: OneQuestionSurveyStrings.excellent
            }
        }

    } // Column: content

}
