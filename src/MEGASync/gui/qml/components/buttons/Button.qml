import Components 1.0
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import Common 1.0

Qml.RoundButton {
    id: button

    /*
     * Properties
     */

    property bool primary: false
    property bool iconRight: false
    property string iconSource: ""

    property alias sourceSize: icon.sourceSize

    /*
     * Component
     */

    bottomPadding: 12
    topPadding: 12

    leftPadding: {
        if(!iconRight) {
            24 + (iconSource.length !== 0 ? icon.sourceSize.width : 0);
        } else {
            24;
        }
    }

    rightPadding: {
        if(iconRight) {
            24 + (iconSource.length !== 0 ? icon.sourceSize.width : 0);
        } else {
            24;
        }
    }

    contentItem: Text {
        id: textItem

        color: primary ? Styles.textOnColor : Styles.buttonPrimary
        text: button.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        font {
            pixelSize: 14
            weight: Font.DemiBold
        }

    } // contentItem -> Text -> textItem

    background: Rectangle {
        width: button.width
        height: button.height

        Rectangle {
            id: rect

            width: parent.width
            height: parent.height
            border.width: 2
            radius: 6
            color: primary ? Styles.buttonPrimary : "transparent"
            border.color: Styles.buttonPrimary
            opacity: button.enabled ? 1.0 : 0.1
        }

        SvgImage {
            id: icon

            source: iconSource
            color: Styles.iconOnColor
            sourceSize.width: 10
            sourceSize.height: 10

            x: {
                if(iconRight) {
                    button.leftPadding + textItem.width + button.rightPadding / 2 - icon.sourceSize.width / 2;
                } else {
                    (button.leftPadding) / 2 - icon.sourceSize.width / 2;
                }
            }

            anchors{
                verticalCenter: rect.verticalCenter
            }

        } // SvgImage -> icon

    } // Rectangle -> rect

    /*
     * Child components
     */

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed: mouse.accepted = false
        cursorShape: Qt.PointingHandCursor
    }

}
