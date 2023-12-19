import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

Qml.MenuItem {
    id: root

    enum Position {
        FIRST = 0,
        INTER = 1,
        LAST = 2
    }

    readonly property int paddingSize: 8

    property int position: MenuItem.Position.INTER

    function getBackgroundColor(){
        if(root.pressed) {
            return Styles.surface2;
        }
        else if(root.hovered) {
            return Styles.textInverse;
        }

        return "transparent";
    }

    width: 200
    height: root.position === MenuItem.Position.FIRST || position === MenuItem.Position.LAST ? 48 : 40
    leftPadding: paddingSize
    rightPadding: paddingSize
    topPadding: root.position === MenuItem.Position.FIRST ? paddingSize : 0
    bottomPadding: root.position === MenuItem.Position.LAST ? paddingSize : 0

    contentItem: Rectangle {
        id: itemBorder

        implicitWidth: 184
        implicitHeight: 40
        color: getBackgroundColor();
        border.color: root.activeFocus ? Styles.focus : "transparent";
        border.width: 4
        radius: 4

        Row {
            id: row

            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 12

            SvgImage {
                id: itemImage

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.topMargin: 12
                anchors.bottomMargin: 12
                source: root.icon.source
                sourceSize: Qt.size(16, 16)
                color: Styles.iconPrimary
            }

            Texts.Text {
                id: itemText

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.topMargin: 10
                anchors.bottomMargin: 10
                verticalAlignment: Text.AlignVCenter
                text: root.text
                font.pixelSize: Texts.Text.Size.Medium
                color: Styles.textPrimary
            }
        }
    }

    background: Rectangle {
        id: itemBackground

        color: "transparent"
    }

    MouseArea {
        id: mouseArea

        anchors.fill: root
        cursorShape: Qt.PointingHandCursor
        onPressed: mouse.accepted = false;
    }
}
