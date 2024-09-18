import QtQuick 2.15

import common 1.0

import components.images 1.0
import components.texts 1.0

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
                    text: UpsellStrings.storageAlmostFullTitle // TODO: Replace by the text for each case (almost/full storage, transfer)
                }

                SecondaryText {
                    id: textItem

                    width: parent.width
                    lineHeight: root.textLineHeight
                    lineHeightMode: Text.FixedHeight
                    urlColor: ColorTheme.textSecondary
                    urlVisitedColor: ColorTheme.textSecondary
                    undelineLink: true
                    url: Links.contact // TODO: Add link to rubish bin
                    rawText: UpsellStrings.storageAlmostFullText // TODO: Replace by the text for each case (almost/full storage, transfer)
                }
            }

        } // Row: contentRow

    } // Rectangle: content

    Component.onCompleted: {
        textItem.forceActiveFocus();
    }
}
