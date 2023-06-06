// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// Local
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Common 1.0

Item {
    id: root

    property NotificationInfo attributes: NotificationInfo {}

    property string title
    property string text

    visible: false
    height: content.height
    Layout.preferredHeight: content.height

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

    Rectangle {
        id: content

        anchors.left: parent.left
        anchors.right: parent.right
        height: mainLayout.height + 2 * attributes.margin
        color: attributes.backgroundColor
        radius: attributes.radius

        Row {
            id: mainLayout

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: attributes.margin
            height: textColumn.height + attributes.margin
            spacing: attributes.spacing

            Loader {
                id: iconLoader
            }

            Column {
                id: textColumn

                height: titleLoader.height + textLoader.height
                width: mainLayout.width - iconLoader.width - mainLayout.spacing
                spacing: attributes.spacing

                Loader {
                    id: titleLoader

                    anchors.left: parent.left
                    anchors.right: parent.right
                }

                Loader {
                    id: textLoader

                    anchors.left: parent.left
                    anchors.right: parent.right
                }
            }
        }
    }

    Component {
        id: iconComponent

        MegaImages.SvgImage {
            color: attributes.iconColor
            source: attributes.icon.source
            sourceSize: attributes.icon.size
        }
    }

    Component {
        id: titleComponent

        MegaTexts.Text {
            text: root.title
            color: attributes.titleColor
            font.bold: true
        }
    }

    Component {
        id: textComponent

        MegaTexts.RichText {
            text: root.text
            color: attributes.textColor
        }
    }

}
