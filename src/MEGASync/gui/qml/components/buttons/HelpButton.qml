import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0
import components.images 1.0

Qml.Button {
    id: button

    property string url
    property size iconSize: Qt.size(16, 16)
    property bool visited: false

    property Colors colors: Colors {}
    property Sizes sizes: Sizes {}

    width: icon.width + spacing + textComponent.width
    height: Math.max(textComponent.height, icon.height)
    Layout.preferredWidth: width
    Layout.preferredHeight: height
    spacing: 6
    opacity: enabled ? 1.0 : 0.3

    function openHelpUrl() {
        Qt.openUrlExternally(url);
        visited = true;
    }

    contentItem: Row {
        anchors.fill: parent
        spacing: button.spacing

        SvgImage {
            id: icon

            anchors.verticalCenter: parent.verticalCenter
            source: Images.helpCircle
            color: visited ? Styles.linkVisited : Styles.linkPrimary
            sourceSize: button.iconSize
        }

        Text {
            id: textComponent

            color: icon.color
            text: button.text
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: Text.Size.Medium
            lineHeight: 20
            lineHeightMode: Text.FixedHeight
        }
    }

    background: Rectangle {
        id: background

        anchors.fill: parent
        anchors {
            topMargin: -sizes.focusBorderWidth
            bottomMargin: -sizes.focusBorderWidth
            rightMargin: -sizes.focusBorderWidth * 2
            leftMargin: -sizes.focusBorderWidth * 2
        }
        color: "transparent"
        border.color: button.activeFocus ? colors.focus : "transparent"
        border.width: sizes.focusBorderWidth
        radius: sizes.focusBorderRadius
    }

    Keys.onPressed: {
        if(event.key === Qt.Key_Space || event.key === Qt.Key_Return) {
            openHelpUrl()
            event.accepted = true
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: {
            openHelpUrl()
        }
    }

}
