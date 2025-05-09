import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0
import components.chips 1.0 as Chips
import components.buttons 1.0

Rectangle {
    id: root

    readonly property real planDefaultWidth: 160
    readonly property real cardRadius: 8
    readonly property real contentMargin: 12
    readonly property real contentSpacing: 8
    readonly property real totalNumContentSpacing: 4
    readonly property real titleLineHeight: 20
    readonly property real priceLineHeight: 30
    readonly property real pricePeriodLineHeight: 18
    readonly property real discountLineHeight: 18
    readonly property real priceSpacing: 0
    readonly property real bottomSpacing: 4
    readonly property real buttonHeight: 26
    readonly property real bottomTextsSpacing: 4
    readonly property real tryProFlexiLineHeight: 16
    readonly property int borderWidthDefault: 1
    readonly property int borderWidthRecommended: 2

    property bool recommended: false
    property bool currentPlan: false
    property bool monthly: false
    property bool billingCurrency: false
    property bool showProFlexiMessage: false
    property bool showOnlyProFlexi: false
    property string currencyName: ""
    property string name: ""
    property string gbStorage: ""
    property string gbTransfer: ""
    property string price: ""
    property string totalPriceWithoutDiscount: ""
    property string monthlyPriceWithDiscount: ""
    property string buttonName: ""

    property string billedText: {
        root.billingCurrency
            ? (root.monthly ? UpsellStrings.perMonth : UpsellStrings.billedYearly)
            : (root.monthly
                ? UpsellStrings.perMonthWithBillingCurrency.arg(root.currencyName)
                : UpsellStrings.billedYearlyWithBillingCurrency.arg(root.currencyName))
    }

    signal buyButtonClicked()

    function getChipBackgroundColor() {
        if (root.currentPlan) {
            return ColorTheme.notificationInfo;
        }
        else if (root.recommended) {
            return ColorTheme.borderBrand;
        }
        else {
            return "transparent";
        }
    }

    function getChipTextColor() {
        if (root.currentPlan) {
            return ColorTheme.textInfo;
        }
        else if (root.recommended) {
            return ColorTheme.textOnColor;
        }
        else {
            return "transparent";
        }
    }

    function updateBilledPeriodText() {
        if (!billedPeriodInfoText.enabled) {
            billedPeriodInfoText.text = root.billedText;
        }
    }

    width: Math.max(contentColumn.implicitWidth, root.planDefaultWidth) + (2 * root.contentMargin)
    height: contentColumn.implicitHeight + (2 * root.contentMargin)

    border {
        width: root.recommended ? root.borderWidthRecommended : root.borderWidthDefault
        color: root.recommended ? ColorTheme.borderBrand : ColorTheme.borderStrong
    }
    radius: root.cardRadius
    color: ColorTheme.pageBackground

    ColumnLayout {
        id: contentColumn

        spacing: root.contentSpacing

        anchors.margins: root.contentMargin
        anchors.fill: parent

        Text {
            id: titleText

            font {
                family: FontStyles.poppinsFontFamily
                pixelSize: Text.Size.LARGE
                weight: Font.DemiBold
            }
            lineHeight: root.titleLineHeight
            lineHeightMode: Text.FixedHeight
            text: root.name
            enabled: root.enabled && !root.showOnlyProFlexi
        }

        Chips.Chip {
            id: recommendedChip

            sizes: Chips.SmallSizes {}
            text: root.currentPlan ? UpsellStrings.currentPlan : UpsellStrings.recommended
            visible: true
            opacity: (root.recommended || root.currentPlan) ? 1.0 : 0.0
            colors {
                background: getChipBackgroundColor()
                border: getChipBackgroundColor()
                text: getChipTextColor()
            }
        }

        ColumnLayout {
            id: priceColumn

            spacing: root.priceSpacing

            SecondaryText {
                id: priceTextWithDiscount

                lineHeight: root.discountLineHeight
                lineHeightMode: Text.FixedHeight
                font.strikeout: true
                text: root.totalPriceWithoutDiscount
                visible: !root.monthly
                enabled: root.enabled && !root.showOnlyProFlexi
            }

            Text {
                id: priceText

                font {
                    family: FontStyles.poppinsFontFamily
                    pixelSize: Text.Size.EXTRA_LARGE
                    weight: Font.Bold
                }
                lineHeight: root.priceLineHeight
                lineHeightMode: Text.FixedHeight
                text: root.price
                visible: root.enabled && !root.showOnlyProFlexi
            }

            SecondaryText {
                id: billedPeriodInfoText

                lineHeight: root.pricePeriodLineHeight
                lineHeightMode: Text.FixedHeight
                visible: root.enabled && !root.showOnlyProFlexi
                text: root.billedText
            }

            SecondaryText {
                id: pricePerMonthText

                lineHeight: root.pricePeriodLineHeight
                lineHeightMode: Text.FixedHeight
                text: UpsellStrings.pricePerMonth.arg(root.monthlyPriceWithDiscount)
                visible: !root.monthly
                enabled: root.enabled && !root.showOnlyProFlexi
            }
        }

        ColumnLayout {
                id: bottomTextsColumn

                spacing: root.showProFlexiMessage ? root.bottomTextsSpacing : 0

                ColumnLayout {
                    id: storageTransferTextColumn

                    enabled: root.enabled && !root.showOnlyProFlexi
                    spacing: root.bottomSpacing

                    Text {
                        id: storageText

                        font.weight: Font.DemiBold
                        text: UpsellStrings.storage.arg(gbStorage)
                        onTextChanged: {
                            // When the component is disabled, the text is not being updated.
                            // Force to update the text when the component is disabled.
                            Qt.callLater(updateBilledPeriodText);
                        }
                    }

                    Text {
                        id: transferText

                        font.weight: Font.DemiBold
                        text: UpsellStrings.transfer.arg(gbTransfer)
                    }
                }

                SecondaryText {
                    id: tryProFlexiText

                    lineHeight: root.tryProFlexiLineHeight
                    lineHeightMode: Text.FixedHeight
                    font {
                        pixelSize: Text.Size.SMALL
                        bold: root.showOnlyProFlexi
                    }
                    color: {
                        if (root.showOnlyProFlexi) {
                            return ColorTheme.textPrimary;
                        }
                        else {
                            return enabled ? ColorTheme.textSecondary : ColorTheme.textDisabled;
                        }
                    }
                    underlineLink: true
                    manageClick: true
                    rawText: UpsellStrings.tryProFlexi
                    visible: root.showProFlexiMessage
                    onLinkClicked: {
                        upsellComponentAccess.linkTryProFlexiClicked();
                    }
                }
            }

/*
        ColumnLayout {
            id: buyButtonContainer

            Layout.preferredWidth: contentColumn.implicitWidth

            PrimaryButton {
                id: buyButton

                sizes {
                    fillWidth: false
                    textFontSize: Text.Size.NORMAL
                }

                Layout.fillWidth: true
                height: root.buttonHeight + 2 * Constants.focusBorderWidth
                text: UpsellStrings.buyPlan.arg(root.buttonName)
                onClicked: {
                    root.buyButtonClicked();
                }
                visible: root.recommended && parent.enabled
            }

            OutlineButton {
                sizes {
                    fillWidth: false
                    textFontSize: Text.Size.NORMAL
                }

                Layout.fillWidth: true
                height: root.buttonHeight + 2 * Constants.focusBorderWidth
                text: UpsellStrings.buyPlan.arg(root.buttonName)
                onClicked: {
                    root.buyButtonClicked();
                }
                visible: !buyButton.visible
                enabled: root.enabled && !root.showOnlyProFlexi
            }
        }
        */
    }
}
