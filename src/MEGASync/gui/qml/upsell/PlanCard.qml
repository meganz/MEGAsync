import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0

RoundButton {
    id: root

    readonly property int borderWidth: 1
    readonly property int contentMargin: 2 * Constants.focusBorderWidth + root.borderWidth + 12
    readonly property int contentBottomMargin: root.contentMargin + 4

    width: 168
    height: 223

    contentItem: Column {
        id: contentColumn

        anchors {
            fill: parent
            topMargin: root.contentMargin
            leftMargin: root.contentMargin
            rightMargin: root.contentMargin
            bottomMargin: root.contentBottomMargin
        }
        spacing: 20

        Column {
            id: topColumn

            anchors {
                left: parent.left
                right: parent.right
            }
            spacing: 12

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
                lineHeight: 30
                lineHeightMode: Text.FixedHeight
                text: "Pro I"
            }

            Rectangle {
                id: recommendedLabel

                anchors.left: parent.left
                width: recommnededText.width + 8
                height: recommnededText.height + 4
                radius: 4
                color: ColorTheme.selectionControl

                Text {
                    id: recommnededText

                    anchors.centerIn: parent
                    font {
                        pixelSize: Text.Size.SMALL
                        weight: Font.DemiBold
                    }
                    color: ColorTheme.textInverseAccent
                    text: "Recommended"
                }
            }

            Column {
                id: priceColumn

                spacing: 0
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
                    lineHeight: 30
                    lineHeightMode: Text.FixedHeight
                    text: "€X.XX*"
                }

                SecondaryText {
                    id: pricePeriodText

                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: "NZD per year"
                }
            }

        } // Column: topColumn

        Column {
            id: bottomColumn

            spacing: 10
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
                text: "2 TB storage"
            }

            Text {
                id: transferText

                anchors {
                    left: parent.left
                    right: parent.right
                }
                font.weight: Font.DemiBold
                text: "24 TB transfer"
            }
        }

    } // Column: contentColumn (contentItem)

    background: Rectangle {
        id: focusRect

        border {
            width: Constants.focusBorderWidth
            color: root.focus ? ColorTheme.focus : "transparent"
        }
        radius: 12

        Rectangle {
            id: contentRect

            anchors {
                fill: parent
                margins: focusRect.border.width
            }
            border {
                width: root.borderWidth
                color: ColorTheme.borderStrongSelected
            }
            radius: 8
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed: { mouse.accepted = false; }
        cursorShape: Qt.PointingHandCursor
    }

}
