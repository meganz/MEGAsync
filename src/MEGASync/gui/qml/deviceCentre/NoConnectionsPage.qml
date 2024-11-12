import QtQuick 2.0

import components.texts 1.0 as Texts
import common 1.0


Item {
    id:root

    readonly property int imageDimension: 128

    anchors.fill: parent

    Image {
        id: image

        anchors{
            bottom: textSection.top
            bottomMargin: 24
            horizontalCenter: parent.horizontalCenter
        }
        width: imageDimension
        height:imageDimension
        sourceSize: Qt.size(imageDimension,imageDimension)
        source: Images.multidevices
    }
    Rectangle{
        id: textSection

        width: 400
        height: title.height + description.height + 12
        anchors.centerIn: parent

        Texts.RichText {
            id: title

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: image.bottom
            anchors.topMargin: 24
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.NoWrap
            font {
                pixelSize: 18
                weight: Font.DemiBold
            }
            lineHeight: 28
            lineHeightMode: Text.FixedHeight
            text: DeviceCentreStrings.noConnectionTitle;
        }

        Texts.RichText {
            id: description

            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: title.bottom
            anchors.topMargin: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font {
                pixelSize: Texts.Text.Size.NORMAL
                weight: Font.DemiBold
            }
            lineHeight: 18
            lineHeightMode: Text.FixedHeight
            wrapMode: Text.WordWrap
            text: DeviceCentreStrings.noConnectionDescription;
        }
    }
}
