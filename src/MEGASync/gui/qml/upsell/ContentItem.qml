import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0
import components.buttons 1.0
import components.chips 1.0 as Chips

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

                    checked: upsellPlansAccess.monthly
                    text: UpsellStrings.billedMonthly
                    ButtonGroup.group: billedPeriodButtonGroupItem

                    onCheckedChanged: {
                        upsellComponentAccess.billedRadioButtonClicked(true);
                    }
                }

                RadioButton {
                    id: billedYearlyRadioButton

                    checked: !upsellPlansAccess.monthly
                    text: UpsellStrings.billedYearly
                    ButtonGroup.group: billedPeriodButtonGroupItem

                    onCheckedChanged: {
                        upsellComponentAccess.billedRadioButtonClicked(false);
                    }
                }
            }

            Chips.Chip {
                id: billedChip

                anchors.verticalCenter: parent.verticalCenter
                sizes: Chips.SmallSizes {}
                text: UpsellStrings.billedSaveUpText.arg(upsellPlansAccess.currentDiscount)
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

            Repeater {
                id: plansRepeater

                model: upsellModelAccess

                PlanCard {
                    id: card

                    name: model.name
                    recommended: model.recommended
                    gbStorage: model.gbStorage
                    gbTransfer: model.gbTransfer
                    price: model.price
                    selected: model.selected

                    ButtonGroup.group: planButtonGroupItem

                    onClicked: {
                        if (!model.selected) {
                            model.selected = true;
                        }
                    }
                }
            }

        } // Row: plansRow

        SecondaryText {
            id: footerText

            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: UpsellStrings.estimatedPrice
            visible: !upsellPlansAccess.billingCurrency
        }

        Row {
            id: footerButtonsRow

            anchors.horizontalCenter: parent.horizontalCenter
            spacing: root.buttonsSpacing

            SecondaryButton {
                id: leftButton

                text: UpsellStrings.notNow
                onClicked: {
                    window.close();
                }
            }

            PrimaryButton {
                id: rightButton

                text: UpsellStrings.buyPlan.arg(upsellPlansAccess.currentPlanName)
                onClicked: {
                    upsellComponentAccess.buyButtonClicked();
                }
            }
        }

    } // Column: columnItem

    ButtonGroup {
        id: billedPeriodButtonGroupItem
    }

    ButtonGroup {
        id: planButtonGroupItem
    }

}
