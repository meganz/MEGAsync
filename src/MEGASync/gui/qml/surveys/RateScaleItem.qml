import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0
import components.buttons 1.0

Item {
    id: root

    property int numRateItems: 5

    height: content.height

    Column {
        id: content

        width: parent.width

        readonly property real defaultLineHeight: 20
        readonly property real contentSpacing: 10

        Row {
            width: parent.width
            height: rateItem.height
            //spacing: content.contentSpacing

            Repeater {
                model: root.numRateItems
                delegate: IconButton {
                    id: rateItem

                    icons.source: Images.helpCircle
                    sizes.iconSize: Qt.size(35, 35)
                    onClicked: {
                        console.log("FERTEST: RateScaleItem clicked " + index);
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
