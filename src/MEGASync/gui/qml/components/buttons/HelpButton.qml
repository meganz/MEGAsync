// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Common 1.0

Qml.Button {
    id: button

    property string url
    property size iconSize: Qt.size(16, 16)
    property bool visited: false

    width: icon.width + spacing + textComponent.width
    height: Math.max(textComponent.height, icon.height)
    Layout.preferredWidth: width
    Layout.preferredHeight: height
    spacing: 6

    contentItem: Row {
        anchors.fill: parent
        spacing: button.spacing

        MegaImages.SvgImage {
            id: icon

            source: Images.helpCircle
            color: visited ? Styles.linkVisited : Styles.linkPrimary
            sourceSize: button.iconSize
        }

        MegaTexts.Text {
            id: textComponent

            anchors.top: parent.top
            color: icon.color
            text: button.text
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: MegaTexts.Text.Size.Medium
        }
    }

    background: Rectangle {
        anchors.fill: parent
        color: "transparent"
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: {
            Qt.openUrlExternally(url);
            visited = true;
        }
    }

}
