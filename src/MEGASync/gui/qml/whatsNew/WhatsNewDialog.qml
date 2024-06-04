import QtQuick 2.15
import QtQuick.Window 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.buttons 1.0

import WhatsNewDialog 1.0

WhatsNewDialog {
    id: window

    readonly property int contentMargins: 48
    readonly property int elementsSpacing: 8

    width: 811
    height: 540
    maximumHeight: 540
    maximumWidth: 811
    minimumHeight: 540
    minimumWidth: 811
    color: ColorTheme.surface1
    title: WhatsNewStrings.whatsNew

    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)

    Rectangle {
        id: content

        color:'transparent'
        anchors {
            fill: parent
            margins: contentMargins
        }
        Texts.Text {
            id: title

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            text: WhatsNewStrings.updates
            color: ColorTheme.textPrimary
            lineHeightMode: Text.FixedHeight
            font{
                pixelSize: Texts.Text.Size.EXTRA_LARGE
                weight: Font.DemiBold
                family: FontStyles.poppinsFontFamily
            }
            lineHeight: 30
        }
        Rectangle{
            id: elementsRect

            anchors{
                left: parent.left
                right: parent.right
                top: title.bottom
                topMargin: contentMargins
                bottom: acceptButton.top
                bottomMargin: contentMargins
            }

            color:'transparent'

            Row{
                id: elementsRow

                anchors.fill: parent
                spacing: elementsSpacing
                UpdatesElement{
                    id: firstElemnt

                    anchors{
                        top: parent.top
                        bottom: parent.bottom
                    }
                    imageSource: Images.rocket
                    titleText: WhatsNewStrings.leftTitle
                    descriptionText: WhatsNewStrings.leftDescription
                }
                UpdatesElement{
                    id: secondElement

                    anchors{
                        top: parent.top
                        bottom: parent.bottom
                    }
                    imageSource: Images.contols
                    titleText: WhatsNewStrings.middleTitle
                    descriptionText: WhatsNewStrings.middleDescription
                }
                UpdatesElement{
                    id: thirdElement

                    anchors{
                        top: parent.top
                        bottom: parent.bottom
                    }
                    imageSource: Images.megaCloud
                    titleText: WhatsNewStrings.rightTitle
                    descriptionText: WhatsNewStrings.rightDescription
                }
            } // Row: elementsRow
        }// Rectangle: elementsRect

        PrimaryButton{
            id: acceptButton

            anchors{
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
            }
            text: WhatsNewStrings.gotIt
            focus: true
            onClicked: window.close();
        }
    } // Rectangle: content
}
