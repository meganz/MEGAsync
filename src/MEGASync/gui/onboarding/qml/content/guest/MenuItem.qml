// System
import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml

// QML common
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Common 1.0

Qml.MenuItem {
    id: root

    enum Position {
        First = 0,
        Inter = 1,
        Last = 2
    }

    readonly property int paddingSize: 8

    property int position: MenuItem.Position.Inter

    function getBackgroundColor(){
        if(root.pressed)
        {
            return Styles.surface2;
        }
        else if(root.hovered)
        {
            return Styles.textInverse;
        }
        return "transparent";
    }

    width: 200
    height: root.position === MenuItem.Position.First || position === MenuItem.Position.Last ? 48 : 40
    leftPadding: paddingSize
    rightPadding: paddingSize
    topPadding: root.position === MenuItem.Position.First ? paddingSize : 0
    bottomPadding: root.position === MenuItem.Position.Last ? paddingSize : 0

    contentItem: Rectangle {
        implicitWidth: 184
        implicitHeight: 40
        color: getBackgroundColor();
        border.color: root.visualFocus ? Styles.focus : "transparent";
        border.width: 4
        radius: 4

        Row {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 12

            MegaImages.SvgImage {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.topMargin: 12
                anchors.bottomMargin: 12
                source: root.icon.source
                sourceSize: Qt.size(16, 16)
                color: Styles.iconPrimary
            }

            MegaTexts.Text {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.topMargin: 10
                anchors.bottomMargin: 10
                verticalAlignment: Text.AlignVCenter
                text: root.text
                font.pixelSize: MegaTexts.Text.Size.Medium
                color: Styles.textPrimary
            }
        }
    }

    background: Rectangle {
        color: "transparent"
    }

    MouseArea {
        id: mouseArea

        anchors.fill: root
        cursorShape: Qt.PointingHandCursor
        onPressed: mouse.accepted = false;
    }
}
