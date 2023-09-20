// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// Local
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Common 1.0

Item {
    id: notificationRoot

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
        if(visible && notificationRoot.time > 0) {
            animationToVisible.start();
            visibilityTimer.start();
        } else if(visibilityTimer.running) {
            visibilityTimer.stop();
        }
    }

    SequentialAnimation {
        id: animationToVisible

        NumberAnimation {
            target: notificationRoot
            property: "visible"
            from: 0
            to: 1
            duration: 0
        }

        NumberAnimation {
            target: notificationRoot
            property: "opacity"
            from: 0
            to: 1
            duration: 200
        }
    }

    SequentialAnimation {
        id: animationToInvisible

        NumberAnimation {
            target: notificationRoot
            property: "opacity"
            from: 1
            to: 0
            duration: 200
        }

        NumberAnimation {
            target: notificationRoot
            property: "visible"
            from: 1
            to: 0
            duration: 0
        }

        onFinished: {
            visibilityTimerFinished();
        }
    }

    Rectangle {
        id: content

        anchors.left: parent.left
        anchors.right: parent.right
        height: mainLayout.height
        color: attributes.backgroundColor
        radius: attributes.radius

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
        }

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
                    if(notificationRoot.title.length !== 0) {
                        h += titleLoader.height;
                    }
                    if(notificationRoot.text.length !== 0) {
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
            text: notificationRoot.title
            color: attributes.titleColor
            font.bold: true
        }
    }

    Component {
        id: textComponent

        MegaTexts.RichText {
            text: notificationRoot.text
            color: attributes.textColor
            url: Links.contact
            manageMouse: true
        }
    }

    Timer {
        id: visibilityTimer

        interval: notificationRoot.time
        running: false
        repeat: false
        onTriggered: {
            animationToInvisible.start();
        }
    }
}
