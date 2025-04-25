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
    readonly property real minimumWidth: 496
    readonly property real itemsSpacing: 22
    readonly property real chipSpacing: 12
    readonly property real plansRowSpacing: 8

    property real totalPlansRowSpacing: root.plansRowSpacing * (upsellPlansAccess.plansCount - 1)
    property real plansWidth: root.planDefaultWidth * upsellPlansAccess.plansCount + root.totalPlansRowSpacing
    property real contentWidth: Math.max(topRow.width, root.plansWidth)

    height: columnItem.height
    width: Math.max(root.minimumWidth, root.contentWidth)

    Column {
        id: columnItem

        width: parent.width
        height: topRow.height + root.itemsSpacing + plansRepeater.maxCardHeight
                + (footerText.visible ? footerText.height + root.itemsSpacing : 0)
        spacing: root.itemsSpacing

        Row {
            id: topRow

            spacing: root.chipSpacing
            height: Math.max(comboBoxRow.height, billedChip.height)
            width: comboBoxRow.width + root.chipSpacing + billedChip.width

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
                        plansRepeater.recalculateCardHeight();
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
                            plansRepeater.recalculateCardHeight();
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

        } // Row: topRow

        Item {
            id: plansItem
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            height: plansRepeater.maxCardHeight

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                height: plansRepeater.maxCardHeight
                spacing: root.plansRowSpacing

                Repeater {
                    id: plansRepeater

                    property real maxCardHeight: 0
                    property var reportedHeights: []
                    property bool heightCalculated: false

                    function recalculateCardHeight() {
                        var newMaxHeight = 0;
                        for (var i = 0; i < plansRepeater.count; i++) {
                            var card = plansRepeater.itemAt(i);
                            newMaxHeight = Math.max(newMaxHeight, card.localHeight);
                        }
                        plansRepeater.maxCardHeight = newMaxHeight;
                    }

                    model: upsellModelAccess

                    onMaxCardHeightChanged: {
                        if (plansRepeater.maxCardHeight !== maxCardHeight) {
                            plansRepeater.maxCardHeight = maxCardHeight;
                        }
                    }

                    Component.onCompleted: {
                        var initialMaxHeight = 0;
                        for (var i = 0; i < plansRepeater.count; i++) {
                            var card = plansRepeater.itemAt(i);
                            initialMaxHeight = Math.max(initialMaxHeight, card.localHeight);
                        }
                        plansRepeater.maxCardHeight = initialMaxHeight;
                    }

                    PlanCard {
                        id: card

                        height: plansRepeater.heightCalculated ? plansRepeater.maxCardHeight : card.localHeight
                        externalMaxHeight: plansRepeater.maxCardHeight

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

                        onReportHeight: (height) => {
                            if (!plansRepeater.heightCalculated) {
                                plansRepeater.reportedHeights[model.index] = height;
                                const reportedCount = plansRepeater.reportedHeights.filter(h => typeof h === "number").length;
                                if (reportedCount === upsellPlansAccess.plansCount) {
                                    const maxHeight = Math.max.apply(null, plansRepeater.reportedHeights);
                                    plansRepeater.maxCardHeight = maxHeight;
                                    plansRepeater.heightCalculated = true;
                                }
                            }
                        }

                        onHeightChanged: {
                            if (height < plansRepeater.maxCardHeight) {
                                plansRepeater.maxCardHeight = height;
                            }
                        }

                        onForceUpdate: {
                            plansRepeater.recalculateCardHeight();
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
