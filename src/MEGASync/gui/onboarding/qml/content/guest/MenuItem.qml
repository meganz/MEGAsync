// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml

// QML common
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Common 1.0

Qml.MenuItem {
    id: menuItem

    enum Position {
        First = 0,
        Inter = 1,
        Last = 2
    }

    readonly property int paddingSize: 8

    property int position: MenuItem.Position.Inter

    function getBackgroundColor(){
        if(menuItem.pressed)
        {
            return Styles.surface2;
        }
        else if(menuItem.hovered)
        {
            return Styles.textInverse;
        }
        return "transparent";
    }

    width: 200
    height: position === MenuItem.Position.First || position === MenuItem.Position.Last ? 48 : 40
    leftPadding: paddingSize
    rightPadding: paddingSize
    topPadding: position === MenuItem.Position.First ? paddingSize : 0
    bottomPadding: position === MenuItem.Position.Last ? paddingSize : 0

    contentItem: Rectangle {
        implicitWidth: 184
        implicitHeight: 40
        color: getBackgroundColor();
        border.color: menuItem.visualFocus ? Styles.focus : "transparent";
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
                source: menuItem.icon.source
                sourceSize: Qt.size(16, 16)
                color: Styles.iconPrimary
            }

            MegaTexts.Text {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.topMargin: 10
                anchors.bottomMargin: 10
                verticalAlignment: Text.AlignVCenter
                text: menuItem.text
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

        anchors.fill: menuItem
        cursorShape: Qt.PointingHandCursor
        onPressed: mouse.accepted = false;
    }

}
