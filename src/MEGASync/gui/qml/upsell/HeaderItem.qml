import QtQuick 2.15

import common 1.0

import components.images 1.0
import components.texts 1.0

import UpsellPlans 1.0

FocusScope {
    id: root

    readonly property real textSpacing: 8
    readonly property real titleLineHeight: 30
    readonly property real textLineHeight: 18
    readonly property real textPadding: 12

    height: content.height

    Item {
        id: content

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: root.textPadding
        }
        width: parent.width - 2 * root.textPadding
        height: columnContent.height + 2 * root.textPadding

        Column {
            id: columnContent

            width: parent.width
            spacing: root.textSpacing

            Text {
                width: parent.width
                font {
                    family: FontStyles.poppinsFontFamily
                    pixelSize: Text.Size.LARGE
                    weight: Font.DemiBold
                }
                lineHeight: root.titleLineHeight
                lineHeightMode: Text.FixedHeight
                text: {
                    switch (upsellPlansAccess.viewMode) {
                        case UpsellPlans.ViewMode.STORAGE_ALMOST_FULL:
                            return UpsellStrings.storageAlmostFullTitle;
                        case UpsellPlans.ViewMode.STORAGE_FULL:
                            return UpsellStrings.storageFullTitle;
                        case UpsellPlans.ViewMode.TRANSFER_EXCEEDED:
                            return UpsellStrings.transferQuotaExceededTitle;
                        default:
                            return "";
                    }
                }
            }

            FocusScope {
                id: textFocusScope

                width: parent.width
                height: textItem.height
                focus: true
                activeFocusOnTab: true

                SecondaryText {
                    id: textItem

                    width: parent.width
                    lineHeight: root.textLineHeight
                    lineHeightMode: Text.FixedHeight
                    underlineLink: true
                    manageClick: true
                    focus: parent.activeFocus
                    rawText: {
                        switch (upsellPlansAccess.viewMode) {
                            case UpsellPlans.ViewMode.STORAGE_ALMOST_FULL:
                            case UpsellPlans.ViewMode.STORAGE_FULL:
                                return UpsellStrings.storageText;
                            case UpsellPlans.ViewMode.TRANSFER_EXCEEDED:
                                if (upsellPlansAccess.isProAccount) {
                                    return UpsellStrings.transferQuotaExceededTextPro;
                                }
                                else {
                                    return UpsellStrings.transferQuotaExceededText
                                                .arg(upsellPlansAccess.transferRemainingTime);
                                }
                            default:
                                return "";
                        }
                    }
                    onLinkClicked: {
                        upsellComponentAccess.linkInDescriptionClicked();
                    }
                    onWidthChanged: {
                        // Corner case:
                        // Force focus border to be updated when the text width changes.
                        if (textFocusScope.activeFocus || textItem.activeFocus) {
                            placeFocusBorder();
                        }
                    }
                }

            } // FocusScope: textFocusScope

        }

    } // Item: content

    Component.onCompleted: {
        textFocusScope.forceActiveFocus();
    }
}
