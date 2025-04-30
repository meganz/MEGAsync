import QtQuick 2.15
import QtQuick.Controls 2.15

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

    property real localHeight: root.calculateContentHeight(root.calculateBottomTextsHeight())
                                + 2 * root.contentMargin + Constants.focusAdjustment
    property real externalMaxHeight: 0

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
    signal reportHeight(real height)
    signal forceUpdate()

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

    function calculateBottomTextsHeight() {
        let currentHeight = storageTransferTextColumn.height
            + (root.showProFlexiMessage ? tryProFlexiText.height : 0)
            + bottomTextsColumn.spacing;
        return Math.max(76, currentHeight);
    }

    function calculateContentHeight(bottomHeight) {
        return titleText.height
                + recommendedChip.height
                + priceColumn.height
                + bottomHeight
                + buyButtonContainer.height
                + root.totalNumContentSpacing * root.contentSpacing;
    }

    function updateBilledPeriodText() {
        if (!billedPeriodInfoText.enabled) {
            billedPeriodInfoText.text = root.billedText;
        }
    }

    width: root.planDefaultWidth
    height: root.localHeight

    border {
        width: root.recommended ? root.borderWidthRecommended : root.borderWidthDefault
        color: root.recommended ? ColorTheme.borderBrand : ColorTheme.borderStrong
    }
    radius: root.cardRadius
    color: ColorTheme.pageBackground

    Column {
        id: contentColumn

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            leftMargin: root.contentMargin
            rightMargin: root.contentMargin
            topMargin: root.contentMargin
            bottomMargin: root.contentMargin + Constants.focusAdjustment
        }
        height: root.calculateContentHeight(bottomTextsColumn.height)
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
            height: (root.monthly ? 0 : priceTextWithDiscount.height)
                        + priceText.height
                        + billedPeriodInfoText.height
                        + (root.monthly ? 0 : pricePerMonthText.height)

            SecondaryText {
                id: priceTextWithDiscount

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
                id: priceText

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

                Binding {
                    target: billedPeriodInfoText
                    property: "text"
                    value: root.billedText
                    when: true
                }
            }

            SecondaryText {
                id: pricePerMonthText

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
            id: bottomTextsColumn

            anchors {
                left: parent.left
                right: parent.right
            }
            height: {
                let diff = (root.externalMaxHeight > root.localHeight)
                            ? root.externalMaxHeight - root.localHeight : 0;
                return root.calculateBottomTextsHeight() + diff;
            }
            spacing: root.showProFlexiMessage ? root.bottomTextsSpacing : 0

            Item {
                id: storageTransferItem

                width: parent.width
                height: parent.height
                            - bottomTextsColumn.spacing
                            - (tryProFlexiText.visible ? tryProFlexiText.height : 0)
                enabled: root.enabled && !root.showOnlyProFlexi

                Column {
                    id: storageTransferTextColumn

                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }
                    height: storageText.height + transferText.height + root.bottomSpacing
                    spacing: root.bottomSpacing
                    enabled: parent.enabled

                    Text {
                        id: storageText

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        font.weight: Font.DemiBold
                        text: UpsellStrings.storage.arg(gbStorage)
                        onTextChanged: {
                            // Force to update the height of the component when the text is changed
                            forceUpdate();

                            // When the component is disabled, the text is not being updated.
                            // Force to update the text when the component is disabled.
                            Qt.callLater(updateBilledPeriodText);
                        }
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
            height: buyButton.height

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
                visible: root.recommended && parent.enabled
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
                enabled: root.enabled && !root.showOnlyProFlexi
            }

        }

    }

    Component.onCompleted: {
        reportHeight(root.height)
    }

}
