import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0
import components.buttons 1.0

FocusScope {
    id: root

    readonly property real minimumWidth: 664
    readonly property real itemsSpacing: 12
    readonly property real radioButtonsSpacing: 28
    readonly property real billedRectHorizontalPadding: 8
    readonly property real billedRectVerticalPadding: 4
    readonly property real billedRectRadius: 4
    readonly property real billedTextLineHeight: 16
    readonly property real buttonsSpacing: 4
    readonly property real plansRowSpacing: 0

    height: columnItem.height
    width: columnItem.width

    Column {
        id: columnItem

        width: Math.max(root.minimumWidth, plansRow.width - 2 * Constants.focusBorderWidth)
        spacing: root.itemsSpacing

        Row {
            id: billedRow

            spacing: root.itemsSpacing
            anchors.horizontalCenter: parent.horizontalCenter
            height: comboBoxRow.height

            Row {
                id: comboBoxRow

                spacing: root.radioButtonsSpacing

                RadioButton {
                    id: billedMonthlyRadioButton

                    checked: true
                    text: UpsellStrings.billedMonthly
                    ButtonGroup.group: buttonGroupItem
                }

                RadioButton {
                    id: billedYearlyRadioButton

                    text: UpsellStrings.billedYearly
                    ButtonGroup.group: buttonGroupItem
                }
            }

            Rectangle {
                id: billedRect

                anchors.verticalCenter: parent.verticalCenter
                width: saveUpText.width + root.billedRectHorizontalPadding
                height: saveUpText.height + root.billedRectVerticalPadding
                radius: root.billedRectRadius
                color: ColorTheme.selectionControl

                Text {
                    id: saveUpText

                    anchors.centerIn: parent
                    verticalAlignment: Text.AlignVCenter
                    font {
                        pixelSize: Text.Size.SMALL
                        weight: Font.DemiBold
                    }
                    lineHeight: root.billedTextLineHeight
                    lineHeightMode: Text.FixedHeight
                    color: ColorTheme.textInverseAccent
                    text: UpsellStrings.billedSaveUpText
                }
            }

        } // Row: billedRow

        Row {
            id: plansRow

            anchors {
                topMargin: Constants.focusAdjustment
                leftMargin: Constants.focusAdjustment
                horizontalCenter: parent.horizontalCenter
            }
            spacing: root.plansRowSpacing

            /*
            TODO: Replace by real model using repeater

            Repeater {
                id: plansRepeater

                model: PlanModel {
                    id: plans
                }

                PlanCard {
                    id: card

                    name: model.name
                    price: model.price
                }

            }
            */

            PlanCard {
                id: card1

                name: "Pro Flexi"
                price: "€X.XX*"
                recommended: true
            }

            PlanCard {
                id: card2

                name: "Pro I"
                price: "€X.XX*"
            }

            PlanCard {
                id: card3

                name: "Pro II"
                price: "€X.XX*"
            }

            PlanCard {
                id: card4

                name: "Pro III"
                price: "€X.XX*"
            }

        } // Row: plansRow

        SecondaryText {
            id: footerText

            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: UpsellStrings.estimatedPrice
        }

        Row {
            id: footerButtonsRow

            anchors.horizontalCenter: parent.horizontalCenter
            spacing: root.buttonsSpacing

            SecondaryButton {
                id: leftButton

                text: UpsellStrings.notNow
            }

            PrimaryButton {
                id: rightButton

                text: UpsellStrings.buyPro
            }
        }

    } // Column: columnItem

    ButtonGroup {
        id: buttonGroupItem
    }

}
