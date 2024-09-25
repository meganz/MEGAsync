import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0
import components.chips 1.0

RoundButton {
    id: root

    readonly property real contentMargin: 2 * Constants.focusBorderWidth + root.borderWidth + 12
    readonly property real contentBottomMargin: root.contentMargin + 4
    readonly property real totalWidth: 168
    readonly property real totalHeight: 223
    readonly property real contentSpacing: 20
    readonly property real topSpacing: 12
    readonly property real titleLineHeight: 30
    readonly property real priceSpacing: 0
    readonly property real priceLineHeight: 30
    readonly property real bottomSpacing: 12
    readonly property real backgroundFocusRadius: 12
    readonly property real backgroundRadius: 8
    readonly property int borderWidth: 1

    property alias name: titleText.text
    //property alias price: priceText.text

    property bool selected: false
    property bool recommended: false
    property string gbStorage: ""
    property string gbTransfer: ""
    property string price: ""

    width: root.totalWidth
    height: root.totalHeight

    contentItem: Column {
        id: contentColumn

        anchors {
            fill: parent
            topMargin: root.contentMargin
            leftMargin: root.contentMargin
            rightMargin: root.contentMargin
            bottomMargin: root.contentBottomMargin
        }
        spacing: root.contentSpacing

        Column {
            id: topColumn

            anchors {
                left: parent.left
                right: parent.right
            }
            spacing: root.topSpacing

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
            }

            Chip {
                id: recommendedChip

                sizes: SmallSizes {}
                text: UpsellStrings.recommended
                visible: true
                opacity: root.recommended ? 1.0 : 0.0
            }

            Column {
                id: priceColumn

                spacing: root.priceSpacing
                anchors {
                    left: parent.left
                    right: parent.right
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
                    text: price
                }

                SecondaryText {
                    id: pricePeriodText

                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: {
                        if (upsellPlansAccess.billingCurrency) {
                            return upsellPlansAccess.monthly
                                    ? UpsellStrings.perMonth
                                    : UpsellStrings.perYear;
                        }
                        else {
                            return upsellPlansAccess.monthly
                                    ? UpsellStrings.perMonthWithBillingCurrency.arg(upsellPlansAccess.currencySymbol)
                                    : UpsellStrings.perYearWithBillingCurrency.arg(upsellPlansAccess.currencySymbol);
                        }
                    }
                }
            }

        } // Column: topColumn

        Column {
            id: bottomColumn

            spacing: root.bottomSpacing
            anchors {
                left: parent.left
                right: parent.right
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

    } // Column: contentColumn (contentItem)

    background: Rectangle {
        id: focusRect

        border {
            width: Constants.focusBorderWidth
            color: root.focus ? ColorTheme.focusColor : "transparent"
        }
        radius: root.backgroundFocusRadius

        Rectangle {
            id: contentRect

            anchors {
                fill: parent
                margins: focusRect.border.width
            }
            border {
                width: root.borderWidth
                color: root.selected
                       ? ColorTheme.borderStrongSelected
                       : ColorTheme.borderStrong
            }
            radius: root.backgroundRadius
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
            root.clicked();
            event.accepted = true;
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed: { mouse.accepted = false; }
        cursorShape: Qt.PointingHandCursor
    }

}
