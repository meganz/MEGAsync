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

        color: getTextColor()
        text: button.text
        opacity: button.enabled ? 1.0 : 0.3
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        font {
            pixelSize: 14
            weight: Font.DemiBold
        }

    } // contentItem -> Text -> textItem

    background: Rectangle {
        id: rect

        border.width: 2
        radius: 6

        color: {
            if(primary) {
                Styles.lightTheme ? "#04101E" : "#F4F4F5";
            } else {
                "transparent";
            }
        }

        border.color: {
            Styles.lightTheme ? "#04101E" : "#F4F4F5";
        }

        SvgImage {
            id: icon

            source: iconSource
            color: getTextColor()
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

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed: mouse.accepted = false
        cursorShape: Qt.PointingHandCursor
    }

    /*
     * Functions
     */
    function getTextColor() {
        if(primary) {
            if(Styles.lightTheme) {
                return "#FAFAFB";
            } else {
                return "#04101E";
            }
        } else {
            if(Styles.lightTheme) {
                return "#04101E";
            } else {
                return "#F4F4F5";
            }
        }
    }
}
