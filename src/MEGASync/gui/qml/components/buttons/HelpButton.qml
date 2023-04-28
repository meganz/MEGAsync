// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Components 1.0
import Common 1.0

Qml.Button {
    id: button

    function getLinkColor() {
        return enabled ? Styles.linkPrimary : Styles.linkInverse;
    }

    property string url
    property size iconSize: text.length == 0 ? Qt.size(24, 24) : Qt.size(16, 16)

    height: Math.max(textLoader.height, icon.height)

    onTextChanged: {
        if(text.length === 0) {
            return;
        }

        textLoader.sourceComponent = textComponent;
    }

    contentItem: Rectangle {

        anchors.left: button.left
        anchors.right: button.right
        anchors.top: button.top
        anchors.bottom: button.bottom
        color: "transparent"

        Rectangle {

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: icon.width + textLoader.anchors.leftMargin + textLoader.width
            color: "transparent"

            SvgImage {
                id: icon

                anchors.left: parent.left
                anchors.top: parent.top
                source: Images.helpCircle
                color: button.text.length === 0 ? Styles.buttonPrimary : getLinkColor()
                sourceSize: button.iconSize
            }

            Loader {
                id: textLoader

                anchors.left: icon.right
                anchors.top: parent.top
                anchors.leftMargin: 6
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onClicked: {
                    Qt.openUrlExternally(url);
                }
            }
        }
    }

    background: Rectangle {
        color: "transparent"
    }

    Component {
        id: textComponent

        Text {
            color: getLinkColor()
            text: button.text
            verticalAlignment: Text.AlignVCenter
            font {
                pixelSize: 14
                weight: Font.Light
                family: "Inter"
                styleName: "Medium"
            }
        }
    }
}
