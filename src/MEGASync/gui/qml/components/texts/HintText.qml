// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// Local
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Common 1.0

Row {
    id: root

    property url icon: ""
    property string title: ""
    property string text: ""

    property HintStyle styles: HintStyle {}
    property int textSize: Text.Size.Normal

    height: visible ? titleLoader.height + textLoader.height : 0
    visible: false
    spacing: iconLoader.width > 0 ? 8 : 0

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
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width - iconLoader.width - root.spacing
        spacing: titleLoader.height > 0 && textLoader.height > 0 ? 2 : 0

        Loader {
            id: titleLoader

            width: parent.width
        }

        Loader {
            id: textLoader

            width: parent.width
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
            font.pixelSize: textSize
        }
    }

    Component {
        id: textComponent

        MegaTexts.Text {
            text: root.text
            color: styles.textColor
            opacity: enabled ? 1.0 : 0.2
            font.pixelSize: textSize
        }
    }
}


