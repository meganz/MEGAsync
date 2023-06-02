// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// Local
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Common 1.0

Rectangle {
    id: root

    property url icon: ""
    property string title: ""
    property string text: ""

    property HintStyle styles: HintStyle {}


    height: visible ? titleLoader.height + textLoader.height : 0
    color: "transparent"
    visible: false

    onIconChanged: {
        if(icon.length === 0) {
            return;
        }

        iconLoader.sourceComponent = iconComponent;
    }

    onTitleChanged: {
        if(title.length === 0) {
            return;
        }

        titleLoader.sourceComponent = titleComponent;
    }

    onTextChanged: {
        if(text.length === 0) {
            return;
        }

        textLoader.sourceComponent = textComponent;
    }

    Loader {
        id: iconLoader
    }

    Column {
        anchors.left: iconLoader.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 8

        spacing: 4

        Loader {
            id: titleLoader

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 0
            anchors.rightMargin: 0
        }

        Loader {
            id: textLoader

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.leftMargin: 0
        }
    }

    Component {
        id: iconComponent

        MegaImages.SvgImage {
            source: icon
            color: styles.iconColor
            sourceSize: Qt.size(16, 16)
            opacity: enabled ? 1.0 : 0.2
        }
    }

    Component {
        id: titleComponent

        MegaTexts.Text {
            text: title
            color: styles.titleColor
            opacity: enabled ? 1.0 : 0.2
            font.bold: true
            font.weight: Font.Light
        }
    }

    Component {
        id: textComponent

        MegaTexts.Text {
            text: root.text
            color: styles.textColor
            opacity: enabled ? 1.0 : 0.2
            font.weight: Font.Light
        }
    }
}


