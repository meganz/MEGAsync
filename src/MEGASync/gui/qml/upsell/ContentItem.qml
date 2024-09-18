import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0
import components.buttons 1.0

FocusScope {
    id: root

    height: columnItem.height

    readonly property real itemsSpacing: 12
    readonly property real radioButtonsSpacing: 28
    readonly property real billedRectHorizontalPadding: 8
    readonly property real billedRectVerticalPadding: 4
    readonly property real billedRectRadius: 4
    readonly property real billedTextLineHeight: 16
    readonly property real buttonsSpacing: 4

    Column {
        id: columnItem

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

                    anchors {
                        left: parent.left
                        top: parent.top
                        leftMargin: root.billedRectHorizontalPadding / 2
                        rightMargin: root.billedRectHorizontalPadding / 2
                        topMargin: root.billedRectVerticalPadding / 2
                        bottomMargin: root.billedRectVerticalPadding / 2
                    }
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
                left: parent.left
                leftMargin: Constants.focusAdjustment
            }
            spacing: 0

            PlanCard {
                id: proLitePlan
            }

            PlanCard {
                id: proIPlan
            }

            PlanCard {
                id: proIIPlan
            }

            PlanCard {
                id: proIIIPlan
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
