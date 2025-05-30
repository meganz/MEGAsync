import QtQuick 2.15

import components.texts 1.0 as Texts

import MessageDialogTextInfo 1.0

Loader {
    id: root

    required property var textInfo

    required property real textLineHeight
    required property real textPixelSize
    required property real textWeight
    property real textLeftPadding : 0

    anchors {
        left: parent.left
        right: parent.right
    }
    visible: root.textInfo ? root.textInfo.text !== "" : false

    sourceComponent: {
        if (!root.textInfo) {
            return null;
        }

        switch (root.textInfo.format) {
            case MessageDialogTextInfo.TextFormat.RICH:
                return richTextComponent;
            case MessageDialogTextInfo.TextFormat.PLAIN:
            default:
                return textComponent;
        }
    }

    Component {
        id: textComponent

        Texts.Text {
            lineHeightMode: Text.FixedHeight
            lineHeight: root.textLineHeight
            wrapMode: Text.Wrap
            font {
                pixelSize: root.textPixelSize
                weight: root.textWeight
            }
            text: root.textInfo ? root.textInfo.text : ""
            leftPadding: textLeftPadding
        }
    }

    Component {
        id: richTextComponent

        Texts.RichText {
            lineHeightMode: Text.FixedHeight
            lineHeight: root.textLineHeight
            wrapMode: Text.Wrap
            font {
                pixelSize: root.textPixelSize
                weight: root.textWeight
            }
            rawText: root.textInfo ? root.textInfo.text : ""

            Component.onCompleted: {
                if (hasLink()) {
                    root.activeFocusOnTab = true;
                    root.focus = true;
                    focus = true;
                }
            }

            leftPadding: textLeftPadding
        }
    }

}
