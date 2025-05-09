import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0
import components.buttons 1.0
import components.chips 1.0 as Chips

import UpsellPlans 1.0

FocusScope {
    id: root

    readonly property real itemsSpacing: 22
    readonly property real chipSpacing: 12
    readonly property real plansRowSpacing: 8

    height: columnItem.height
    width: columnItem.width

    Column {
        id: columnItem

        spacing: root.itemsSpacing

        Row {
            id: topRow

            spacing: root.chipSpacing

            Row {
                id: comboBoxRow

                spacing: root.itemsSpacing

                RadioButton {
                    id: billedMonthlyRadioButton

                    checked: upsellPlansAccess.monthly
                    text: UpsellStrings.billedMonthly
                    ButtonGroup.group: billedPeriodButtonGroupItem

                    onUserChecked: {
                        upsellComponentAccess.billedRadioButtonClicked(true);
                    }
                }

                RadioButton {
                    id: billedYearlyRadioButton

                    checked: !upsellPlansAccess.monthly
                    text: UpsellStrings.billedYearly
                    ButtonGroup.group: billedPeriodButtonGroupItem
                    enabled: !upsellPlansAccess.onlyProFlexiAvailable

                    onUserChecked: {
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
                visible: upsellPlansAccess.currentDiscount > 0 && !upsellPlansAccess.onlyProFlexiAvailable
            }

        }

        Item {
            id: plansItem

            anchors.horizontalCenter: parent.horizontalCenter
            width: rowLayout.implicitWidth
            height: rowLayout.implicitHeight

            RowLayout {
                id: rowLayout

                anchors.horizontalCenter: parent.horizontalCenter
                spacing: root.plansRowSpacing

                Repeater {
                    id: plansRepeater

                    model: upsellModelAccess

                    PlanCard {
                        id: card

                        Layout.preferredHeight: rowLayout.height
                        Layout.minimumHeight: height
                        Layout.preferredWidth: width

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
                            upsellComponentAccess.buyButtonClicked(index);
                        }
                    }
                }

            }
        }

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
