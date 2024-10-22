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

                    property color colorChecked: rateItem.enabled && (rateItem.checked || rateItem.hovered)
                                                 ? ColorTheme.supportWarning
                                                 : ColorTheme.iconDisabled

                    icons {
                        colorEnabled: rateItem.colorChecked
                        colorDisabled: rateItem.colorChecked
                        colorHovered: rateItem.colorChecked
                        colorPressed: rateItem.colorChecked
                        manageSource: true
                        sourcePressed: Images.starFilled
                        sourceHovered: Images.starFilled
                        sourceDisabled: rateItem.checked ? Images.starFilled : Images.starEmpty
                        sourceEnabled: Images.starEmpty
                    }
                    colors.pressed: "transparent"
                    sizes.iconSize: Qt.size(34, 33)
                    onClicked: {
                        score = index + 1;
                        for (let i = 0; i < root.numRateItems; i++) {
                            rateRepeater.itemAt(i).checked = i <= (score - 1);
                        }
                    }

                    onHoveredChanged: {
                        if (rateItem.hovered) {
                            for (let j = 0; j < root.numRateItems; j++) {
                                rateRepeater.itemAt(j).focus = false;
                                rateRepeater.itemAt(j).checked = j <= index;
                            }
                        }
                        else {
                            for (let k = 0; k < root.numRateItems; k++) {
                                rateRepeater.itemAt(k).checked = k <= (score - 1);
                            }
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
