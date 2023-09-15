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

    property string title: ""
    property string text: ""
    property int time: 0

    signal visibilityTimerFinished

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

    onVisibleChanged: {
        if(visible && root.time > 0) {
            visibilityTimer.start();
        } else if(visibilityTimer.running) {
            visibilityTimer.stop();
        }
    }

    Rectangle {
        id: content

        anchors.left: parent.left
        anchors.right: parent.right
        height: mainLayout.height
        color: attributes.backgroundColor
        radius: attributes.radius

        Row {
            id: mainLayout

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: attributes.margin
            height: textColumn.height > 0 ? textColumn.height + 2 * attributes.margin : 0
            spacing: attributes.spacing

            Loader {
                id: iconLoader
            }

            Column {
                id: textColumn

                height: {
                    var h = 0;
                    if(root.title.length !== 0) {
                        h += titleLoader.height;
                    }
                    if(root.text.length !== 0) {
                        h += textLoader.height;
                    }
                    return h;
                }
                width: mainLayout.width - iconLoader.width - mainLayout.spacing

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

    Rectangle {
        height: attributes.radius
        color: attributes.backgroundColor
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        visible: attributes.topBorderRect
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

        MegaTexts.RichText {
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
            url: Links.contact
            manageMouse: true
        }
    }

    Timer {
        id: visibilityTimer

        interval: root.time
        running: false
        repeat: false
        onTriggered: {
            visibilityTimerFinished();
        }
    }
}
