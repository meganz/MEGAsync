import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0
import components.buttons 1.0
import components.chips 1.0 as Chips

import UpsellPlans 1.0

FocusScope {
    id: root

    readonly property real planDefaultWidth: 160
    readonly property real planDefaultHeight: upsellPlansAccess.monthly ? 246 : 283
    readonly property real minimumWidth: 496
    readonly property real itemsSpacing: 22
    readonly property real chipSpacing: 12
    readonly property real plansRowSpacing: 8

    property real plansWidth: root.planDefaultWidth * upsellPlansAccess.plansCount
                                + root.plansRowSpacing * (upsellPlansAccess.plansCount - 1)

    height: columnItem.height
    width: Math.max(root.minimumWidth, root.plansWidth)

    Column {
        id: columnItem

        width: parent.width
        height: topRow.height + root.itemsSpacing + root.planDefaultHeight
                + (footerText.visible ? footerText.height + root.itemsSpacing : 0)
        spacing: root.itemsSpacing

        Row {
            id: topRow

            anchors {
                left: parent.left
                leftMargin: root.chipSpacing + Constants.focusAdjustment
            }
            spacing: root.chipSpacing
            height: Math.max(comboBoxRow.height, billedChip.height)

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
                    enabled: !upsellPlansAccess.onlyProFlexiAvailable

                    onCheckedChanged: {
                        if (billedYearlyRadioButton.enabled) {
                            upsellComponentAccess.billedRadioButtonClicked(false);
                        }
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

        } // Row: topRow

        Item {
            id: plansItem

            anchors {
                horizontalCenter: parent.horizontalCenter
                topMargin: Constants.focusAdjustment
                leftMargin: Constants.focusAdjustment
            }
            width: parent.width
            height: root.planDefaultHeight

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                height: plansRepeater.height
                spacing: root.plansRowSpacing

                Repeater {
                    id: plansRepeater

                    model: upsellModelAccess

                    PlanCard {
                        id: card

                        width: root.planDefaultWidth
                        height: root.planDefaultHeight

                        name: model.name
                        buttonName: model.buttonName
                        recommended: model.recommended
                        currentPlan: model.currentPlan
                        gbStorage: model.gbStorage
                        gbTransfer: model.gbTransfer
                        price: model.price
                        totalPriceWithoutDiscount: model.totalPriceWithoutDiscount
                        monthlyPriceWithDiscount: model.monthlyPriceWithDiscount
                        enabled: model.available || model.showOnlyProFlexi
                        showProFlexiMessage: model.showProFlexiMessage
                        showOnlyProFlexi: model.showOnlyProFlexi
                        visible: model.display
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
