import QtQuick 2.15

import common 1.0

import components.images 1.0
import components.texts 1.0

import UpsellPlans 1.0

FocusScope {
    id: root

    readonly property real contentBorder: 6
    readonly property real contentLeftMargin: 24
    readonly property real contentRightMargin: 32
    readonly property real contentSpacing: 40
    readonly property real textSpacing: 6
    readonly property real titleLineHeight: 30
    readonly property real textLineHeight: 18
    readonly property int contentHeight: 168
    readonly property int imageWidth: 104

    height: content.height

    Rectangle {
        id: content

        width: parent.width
        height: root.contentHeight
        radius: root.contentBorder
        color: ColorTheme.surface2

        Row {
            id: contentRow

            anchors {
                fill: parent
                verticalCenter: parent.verticalCenter
                leftMargin: root.contentLeftMargin
                rightMargin: root.contentRightMargin
            }
            spacing: root.contentSpacing

            SvgImage {
                id: imageItem

                anchors.verticalCenter: parent.verticalCenter
                sourceSize: Qt.size(root.imageWidth, root.imageWidth)
                source: Images.warning
            }

            Column {
                id: titleColumn

                anchors.verticalCenter: parent.verticalCenter
                width: contentRow.width - imageItem.width - contentRow.spacing
                spacing: root.textSpacing

                Text {
                    id: titleItem

                    width: parent.width
                    font {
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
                        urlColor: ColorTheme.textSecondary
                        underlineLink: true
                        manageClick: true
                        focus: parent.activeFocus
                        rawText: {
                            switch (upsellPlansAccess.viewMode) {
                                case UpsellPlans.ViewMode.STORAGE_ALMOST_FULL:
                                case UpsellPlans.ViewMode.STORAGE_FULL:
                                    return UpsellStrings.storageText;
                                case UpsellPlans.ViewMode.TRANSFER_EXCEEDED:
                                    return UpsellStrings.transferQuotaExceededText
                                                .arg(upsellPlansAccess.transferRemainingTime);
                                default:
                                    return "";
                            }
                        }
                        onLinkClicked: {
                            upsellComponentAccess.linkInDescriptionClicked();
                            switch (upsellPlansAccess.viewMode) {
                                case UpsellPlans.ViewMode.STORAGE_ALMOST_FULL:
                                case UpsellPlans.ViewMode.STORAGE_FULL:
                                    upsellComponentAccess.rubbishLinkClicked();
                                    break;
                                case UpsellPlans.ViewMode.TRANSFER_EXCEEDED:
                                    Qt.openUrlExternally(Links.aboutTransferQuota);
                                    break;
                                default:
                                    break;
                            }
                        }
                        onWidthChanged: {
                            // Corner case:
                            // Force focus border to be updated when the text width changes.
                            if (textFocusScope.activeFocus || textItem.activeFocus) {
                                placeFocusBorder();
                            }
                        }
                    }

                }
            }

        } // Row: contentRow

    } // Rectangle: content

    Component.onCompleted: {
        textFocusScope.forceActiveFocus();
    }
}
