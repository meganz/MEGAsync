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

    width: root.planDefaultWidth
    height: contentColumn.implicitHeight

    border {
        width: root.recommended ? root.borderWidthRecommended : root.borderWidthDefault
        color: root.recommended ? ColorTheme.borderBrand : ColorTheme.borderStrong
    }
    radius: root.cardRadius
    color: ColorTheme.pageBackground

    ColumnLayout {
        id: contentColumn

        anchors {
            left: parent.left
            right: parent.right

            leftMargin: root.contentMargin
            rightMargin: root.contentMargin
            topMargin: root.contentMargin
            bottomMargin: root.contentMargin //+ Constants.focusAdjustment
        }

        spacing: root.contentSpacing

/*
        Rectangle
        {
            color: "green"

            width: 50
            height: 50
        }
        */

        Text {
            id: titleText

            //width: parent.width
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

        /*
        Rectangle
        {
            color: "blue"

            width: 50
            height: 50
        }
        */


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

    }
}
