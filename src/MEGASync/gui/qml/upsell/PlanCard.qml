import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0
import components.chips 1.0 as Chips
import components.buttons 1.0

Rectangle {
    id: root

    readonly property real cardRadius: 8
    readonly property real contentMargin: 12
    readonly property real contentSpacing: 8
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

    signal buyButtonClicked()

    function getChipBackgroundColor() {
        if (root.currentPlan) {
            return ColorTheme.notificationInfo;
        }
        else if (root.recommended) {
            return ColorTheme.borderInteractive;
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

    function getBilledPeriodInfoText() {
        if (root.billingCurrency) {
            return root.monthly
                    ? UpsellStrings.perMonth
                    : UpsellStrings.billedYearly;
        }
        else {
            return root.monthly
                    ? UpsellStrings.perMonthWithBillingCurrency
                        .arg(root.currencyName)
                    : UpsellStrings.billedYearlyWithBillingCurrency
                        .arg(root.currencyName);
        }
    }

    border {
        width: root.recommended ? root.borderWidthRecommended : root.borderWidthDefault
        color: root.recommended ? ColorTheme.borderInteractive : ColorTheme.borderStrong
    }
    radius: root.cardRadius
    color: ColorTheme.pageBackground

    onMonthlyChanged: {
        // When the component is disabled, the text is not being updated.
        // Force to update the text when the component is disabled.
        if (root.monthly && !root.enabled) {
            billedPeriodInfoText.text = getBilledPeriodInfoText();
        }
    }

    Column {
        anchors {
            fill: parent
            leftMargin: root.contentMargin
            rightMargin: root.contentMargin
            topMargin: root.contentMargin
            bottomMargin: root.contentMargin + Constants.focusAdjustment
        }
        spacing: root.contentSpacing

        Text {
            id: titleText

            anchors {
                left: parent.left
                right: parent.right
            }
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

        Column {
            id: priceColumn

            spacing: root.priceSpacing
            anchors {
                left: parent.left
                right: parent.right
            }

            SecondaryText {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                lineHeight: root.discountLineHeight
                lineHeightMode: Text.FixedHeight
                font.strikeout: true
                text: root.totalPriceWithoutDiscount
                visible: !root.monthly
                enabled: root.enabled && !root.showOnlyProFlexi
            }

            Text {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                font {
                    family: FontStyles.poppinsFontFamily
                    pixelSize: Text.Size.EXTRA_LARGE
                    weight: Font.Bold
                }
                lineHeight: root.priceLineHeight
                lineHeightMode: Text.FixedHeight
                text: root.price
                enabled: root.enabled && !root.showOnlyProFlexi
            }

            SecondaryText {
                id: billedPeriodInfoText

                anchors {
                    left: parent.left
                    right: parent.right
                }
                lineHeight: root.pricePeriodLineHeight
                lineHeightMode: Text.FixedHeight
                enabled: root.enabled && !root.showOnlyProFlexi
                text: getBilledPeriodInfoText()
            }

            SecondaryText {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                lineHeight: root.pricePeriodLineHeight
                lineHeightMode: Text.FixedHeight
                text: UpsellStrings.pricePerMonth.arg(root.monthlyPriceWithDiscount)
                visible: !root.monthly
                enabled: root.enabled && !root.showOnlyProFlexi
            }
        }

        Column {
            anchors {
                left: parent.left
                right: parent.right
            }
            height: parent.height - titleText.height - recommendedChip.height
                        - priceColumn.height - buyButtonContainer.height - 4 * root.contentSpacing
            spacing: root.bottomTextsSpacing

            Item {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                height: parent.height - root.bottomTextsSpacing
                            - (tryProFlexiText.visible ? tryProFlexiText.height : 0)
                enabled: root.enabled && !root.showOnlyProFlexi

                Column {
                    spacing: root.bottomSpacing
                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }

                    Text {
                        id: storageText

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        font.weight: Font.DemiBold
                        text: UpsellStrings.storage.arg(gbStorage)
                    }

                    Text {
                        id: transferText

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        font.weight: Font.DemiBold
                        text: UpsellStrings.transfer.arg(gbTransfer)
                    }
                }
            }

            SecondaryText {
                id: tryProFlexiText

                anchors {
                    left: parent.left
                    right: parent.right
                }
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

        Column {
            id: buyButtonContainer

            anchors {
                left: parent.left
                right: parent.right
            }

            PrimaryButton {
                id: buyButton

                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Constants.focusAdjustment
                }
                sizes {
                    fillWidth: true
                    textFontSize: Text.Size.NORMAL
                }
                height: root.buttonHeight + 2 * Constants.focusBorderWidth
                text: UpsellStrings.buyPlan.arg(root.buttonName)
                onClicked: {
                    root.buyButtonClicked();
                }
                visible: root.recommended && root.enabled
            }

            OutlineButton {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Constants.focusAdjustment
                }
                sizes {
                    fillWidth: true
                    textFontSize: Text.Size.NORMAL
                }
                height: root.buttonHeight + 2 * Constants.focusBorderWidth
                text: UpsellStrings.buyPlan.arg(root.buttonName)
                onClicked: {
                    root.buyButtonClicked();
                }
                visible: !buyButton.visible
            }

        }

    }

}
