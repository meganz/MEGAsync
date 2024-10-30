import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts

ColumnLayout {
    id: root

    property ButtonGroup buttonGroup
    property string buttonLabel
    property string description

    property alias checked: optionButton.checked

    spacing: 0

    RadioButton {
        id: optionButton

        indicator: Rectangle {
            id: parentIndicator

            border {
                width: 2
                color: ColorTheme.buttonPrimary
            }

            width: 16
            height: 16
            radius: width / 2
            color: "transparent"

            Rectangle {

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    verticalCenter: parent.verticalCenter
                }

                width: 8
                height: 8

                radius: width / 2
                color: ColorTheme.buttonPrimary
                visible: optionButton.checked
            }
        }

        contentItem: ColumnLayout {

            spacing: 0

            anchors {
                left: parentIndicator.right
                top: parentIndicator.verticalCenter
                leftMargin: 8
                topMargin: -8
            }

            Texts.Text {
                id: mainLabel

                text: buttonLabel
                font {
                    pixelSize: Texts.Text.Size.NORMAL
                    weight: Font.DemiBold
                }
            }

            Texts.SecondaryText {
                text: description
                font {
                    pixelSize: Texts.Text.Size.NORMAL
                    weight: Font.Normal
                }
            }
        }

        checked: true
        ButtonGroup.group: buttonGroup
    }
}
