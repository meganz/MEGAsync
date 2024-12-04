import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0
import components.buttons 1.0
import components.chips 1.0 as Chips

import UpsellPlans 1.0

FocusScope {
    id: root

    readonly property real minimumWidth: 496
    readonly property real itemsSpacing: 22
    readonly property real chipSpacing: 12
    readonly property real plansRowSpacing: 8

    height: columnItem.height
    width: columnItem.width

    Column {
        id: columnItem

        width: Math.max(root.minimumWidth, plansItem.width)
        spacing: root.itemsSpacing

        Row {
            anchors {
                left: parent.left
                leftMargin: root.chipSpacing + Constants.focusAdjustment
            }
            spacing: root.chipSpacing
            height: comboBoxRow.height

            Row {
                id: comboBoxRow

                spacing: root.itemsSpacing

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
                colors {
                    background: ColorTheme.notificationInfo
                    border: ColorTheme.notificationInfo
                    text: ColorTheme.textInfo
                }
                text: UpsellStrings.billedSaveUpText.arg(upsellPlansAccess.currentDiscount)
                visible: upsellPlansAccess.currentDiscount > 0
            }

        }

        Item {
            id: plansItem

            readonly property real planTotalWidth: 160
            readonly property real planTotalHeight: 283

            anchors {
                horizontalCenter: parent.horizontalCenter
                topMargin: Constants.focusAdjustment
                leftMargin: Constants.focusAdjustment
            }
            height: plansItem.planTotalHeight

            Row {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                height: plansRepeater.height
                spacing: root.plansRowSpacing

                onWidthChanged: {
                    // The width depends on the total number of plans, in some cases the number
                    // of plans is different for monthly/yearly billing.
                    plansItem.width = plansItem.planTotalWidth * upsellModelAccess.rowCount()
                                        + root.plansRowSpacing * (upsellModelAccess.rowCount() - 1);
                }

                Repeater {
                    id: plansRepeater

                    model: upsellModelAccess

                    PlanCard {
                        id: card

                        width: plansItem.planTotalWidth
                        height: plansItem.planTotalHeight

                        name: model.name
                        recommended: model.recommended
                        gbStorage: model.gbStorage
                        gbTransfer: model.gbTransfer
                        price: model.price
                        totalPriceWithoutDiscount: model.totalPriceWithoutDiscount
                        monthlyPriceWithDiscount: model.monthlyPriceWithDiscount
                        visible: model.available
                        showProFlexiMessage: model.showProFlexiMessage
                        monthly: upsellPlansAccess.monthly
                        billingCurrency: upsellPlansAccess.billingCurrency
                        currencyName: upsellPlansAccess.currencyName

                        onBuyButtonClicked: {
                            upsellComponentAccess.buyButtonClicked(model.index);
                        }
                    }
                }

            }

        } // Item: plansItem

        SecondaryText {
            id: footerText

            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: UpsellStrings.estimatedPrice
            visible: !upsellPlansAccess.billingCurrency
        }

    } // Column: columnItem

    ButtonGroup {
        id: billedPeriodButtonGroupItem
    }   

}
