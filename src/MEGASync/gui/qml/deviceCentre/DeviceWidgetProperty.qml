import QtQuick 2.0

import common 1.0

import components.images 1.0
import components.texts 1.0 as Texts

Item {
    id:root

    property string name
    property string value

    property alias icon: icon.source

    width: 50
    height: 40

    Texts.RichText {
        id: nameText

        font {
            pixelSize: Texts.Text.Size.SMALL
            weight: Font.DemiBold
        }
        color: ColorTheme.textSecondary
        wrapMode: Text.NoWrap

        text: name;
    }

    SvgImage {
        id: icon

        anchors {
            left: nameText.left
            top: nameText.bottom
        }

        sourceSize: Qt.size(16, 16)

        onSourceChanged: {
            if (source === "") {
                icon.visible = false
                valueText.anchors.leftMargin = 0
            } else {
                icon.visible = true
                valueText.anchors.leftMargin = 4
            }
        }
    }

    Texts.RichText {
        id: valueText

        anchors {
            left: icon.right
            top: nameText.bottom
        }

        font {
            pixelSize: Texts.Text.Size.NORMAL
            weight: Font.Normal
        }
        color: ColorTheme.textSecondary
        wrapMode: Text.NoWrap

        text: value;
    }
}
