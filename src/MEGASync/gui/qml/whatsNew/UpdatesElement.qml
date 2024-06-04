import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts

Item{
    id: root

    property alias imageSource: image.source
    property alias titleText: title.text
    property alias descriptionText: secondaryText.text

    readonly property int imageDimension: 120
    readonly property int majorSpacing: 24
    readonly property int minorSpacing: 12

    width: 233

    Image {
        id: image

        anchors{
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
        width: imageDimension
        height:imageDimension
        sourceSize: Qt.size(imageDimension,imageDimension)
    }

    Texts.RichText{
        id: title

        width: parent.width
        anchors{
            top: image.bottom
            topMargin: majorSpacing
            horizontalCenter: parent.horizontalCenter
        }
        font{
            weight: Font.Bold
            pixelSize: Texts.Text.Size.MEDIUM_LARGE
        }
        color: ColorTheme.textPrimary
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        lineHeightMode: Text.FixedHeight
        lineHeight: 24
    }

    Texts.SecondaryText{
        id: secondaryText

        width: parent.width
        anchors{
            top: title.bottom
            topMargin: minorSpacing
            horizontalCenter: parent.horizontalCenter
        }
        font{
            weight: Font.Normal
            pixelSize: Texts.Text.Size.NORMAL
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: ColorTheme.textSecondary
        lineHeightMode: Text.FixedHeight
        lineHeight: 12
    }
} // Rectangle: root
