// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

Qml.ToolTip {
    id: tooltipRoot

    property url leftIconSource: ""
    property int contentSpacing: 4

    onLeftIconSourceChanged: {
        if(leftIconSource === "") {
            return;
        }

        contentSpacing = padding;
        leftIconLoader.sourceComponent = leftIcon;
    }

    z: 10
    padding: 4

    background: Rectangle {
        anchors.fill: parent
        color: Styles.buttonPrimary
        radius: 4

        Loader {
            id: leftIconLoader

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: tooltipRoot.padding
        }
    }


    contentItem: Item {
        implicitWidth: textToolTip.width + leftIconLoader.width + contentSpacing
        implicitHeight: Math.max(leftIconLoader.width, textToolTip.height)

        MegaTexts.Text {
            id: textToolTip

            property int maxWidth: 778 - leftIconLoader.width - contentSpacing

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: leftIconLoader.width + contentSpacing
            text: tooltipRoot.text
            color: Styles.textInverse
            wrapMode: Text.Wrap
            width: Math.min(textMetrics.width + contentSpacing, maxWidth)
            lineHeight: Math.max(leftIconLoader.height, textMetrics.height)
            lineHeightMode: Text.FixedHeight
            verticalAlignment: Qt.AlignVCenter

            TextMetrics {
                id: textMetrics

                font: textToolTip.font
                text: textToolTip.text
            }
        }

    }

    Component {
        id: leftIcon

        MegaImages.SvgImage {
            source: leftIconSource
            color: Styles.iconOnColor
            sourceSize: Qt.size(16, 16)
        }
    }

}
