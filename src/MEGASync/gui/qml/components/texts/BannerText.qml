import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts

Item {
    id: root

    property alias icon: hint.icon
    property alias title: hint.title
    property alias text: hint.text
    property alias iconColor: hint.iconColor
    property alias titleColor: hint.titleColor
    property alias textColor: hint.textColor
    property alias backgroundColor: backgroundRect.color

    property int type: Constants.MessageType.NONE
    property int margin: 12
    property int radius: 8
    property int borderWidth: 1
    property bool topBorderRect: false
    property bool showBorder: false

    height: backgroundRect.height
    Layout.preferredHeight: backgroundRect.height
    visible: false

    onTypeChanged: {
        hint.type = type;
    }

    function getBackgroundRectColor()
    {
        switch(type) {
            case Constants.MessageType.NONE:
            case Constants.MessageType.SUCCESS:
            case Constants.MessageType.INFO:
                console.warn("BannerText: Constants.MessageType -> " + type + " not defined yet");
                break;
            case Constants.MessageType.WARNING:
                return ColorTheme.notificationWarning;
            case Constants.MessageType.ERROR:
                return ColorTheme.notificationError;
            default:
                console.error("BannerText: Constants.MessageType -> " + type + " does not exist");
                break;
        }
    }

    Rectangle {
        id: backgroundRect

        anchors {
            left: parent.left
            right: parent.right
        }
        height: hint.height + 2 * root.margin
        radius: root.radius
        border {
            color: ColorTheme.borderSubtle
            width: root.showBorder ? root.borderWidth : 0
        }

        color: getBackgroundRectColor();

        MouseArea {
            id: backgroundMouseArea

            anchors.fill: parent
            hoverEnabled: true
        }

        Texts.HintText {
            id: hint

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: root.margin
            }
            textSpacing: 4
        }
    }

    Rectangle {
        id: topBorderRectangle

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: root.radius
        color: backgroundRect.color
        visible: root.topBorderRect
    }

}
