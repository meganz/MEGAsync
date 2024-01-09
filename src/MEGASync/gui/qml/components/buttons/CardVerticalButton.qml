import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

CardButton {
    id: root

    readonly property int textSpacing: 5
    readonly property int textTopMargin: 24
    readonly property int textLineHeight: 16

    property bool useMaxSiblingHeight: false
    property int textHorizontalExtraMargin: 0
    property int contentMargin: 13
    property int contentSpacing: 24
    property int calculatedHeight: imageButton.height + titleText.height + descriptionText.height
                                        + textSpacing + contentSpacing + contentMargin * 2;

    function getHeight() {
        let myHeight = calculatedHeight;
        if(useMaxSiblingHeight) {
            for(var i = 0; i < parent.children.length; i++) {
                if(parent.children[i] !== root && parent.children[i].calculatedHeight > myHeight) {
                    myHeight = parent.children[i].height;
                }
            }
        }
        return myHeight;
    }

    height: getHeight()

    Column {
        id: mainColumn

        anchors {
            fill: parent
            margins: contentMargin
            bottomMargin: 50
        }
        spacing: contentSpacing

        SvgImage {
            id: imageButton

            source: imageSource
            sourceSize: imageSourceSize
        }

        Column {
            id: textsColumn

            anchors {
                left: parent.left
                right: parent.right
            }
            spacing: textSpacing

            Texts.Text {
                id: titleText

                anchors {
                    left: parent.left
                    right: parent.right
                    topMargin: textTopMargin
                    leftMargin: root.textHorizontalExtraMargin
                    rightMargin: root.textHorizontalExtraMargin
                }
                font {
                    pixelSize: Texts.Text.Size.MEDIUM_LARGE
                    weight: Font.Bold
                }
                text: title
                wrapMode: Text.Wrap
            }

            Texts.Text {
                id: descriptionText

                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: root.textHorizontalExtraMargin
                    rightMargin: root.textHorizontalExtraMargin
                }
                font.pixelSize: Texts.Text.Size.SMALL
                color: Styles.textSecondary
                lineHeight: textLineHeight
                lineHeightMode: Text.FixedHeight
                text: description
                wrapMode: Text.Wrap
            }
        }

    } // Column: mainColumn

}
