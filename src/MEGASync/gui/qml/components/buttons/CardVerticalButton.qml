import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

CardButton {
    id: button

    property int textHorizontalExtraMargin: 0
    property int contentMargin: 13
    property int contentSpacing: 24
    property bool useMaxSiblingHeight: false

    readonly property int textSpacing: 5
    readonly property int textTopMargin: 24
    readonly property int textLineHeight: 16

    property int calculatedHeight: imageButton.height + titleText.height + descriptionText.height + textSpacing + contentSpacing + contentMargin * 2;

    function getHeight()
    {
        let myHeight = calculatedHeight;
        if(useMaxSiblingHeight)
        {
            for(var i = 0; i < parent.children.length; i++) {
                if(parent.children[i] !== button && parent.children[i].calculatedHeight > myHeight)
                {
                    myHeight = parent.children[i].height;
                }
            }
        }
        return myHeight;
    }

    height: getHeight();

    Column {
        anchors.fill: parent
        anchors.margins: contentMargin
        anchors.bottomMargin: 50
        spacing: contentSpacing

        SvgImage {
            id: imageButton

            source: imageSource
            sourceSize: imageSourceSize
        }

        Column {
            spacing: textSpacing
            anchors.left: parent.left
            anchors.right: parent.right

            Texts.Text {
                id: titleText

                text: title
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: textTopMargin
                anchors.leftMargin: button.textHorizontalExtraMargin
                anchors.rightMargin: button.textHorizontalExtraMargin
                font.pixelSize: Texts.Text.Size.MediumLarge
                font.weight: Font.Bold
            }

            Texts.Text {
                id: descriptionText

                text: description
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: button.textHorizontalExtraMargin
                anchors.rightMargin: button.textHorizontalExtraMargin
                font.pixelSize: Texts.Text.Size.Small
                color: Styles.textSecondary
                lineHeight: textLineHeight
                lineHeightMode: Text.FixedHeight
            }
        }
    }

//    Component.onCompleted: {
//        for(var i = 0; i < parent.children.length; i++) {
//            if(parent.children[i].height > height)
//            {
//                height = parent.children[i].height;
//            }
//        }
//    }
}
