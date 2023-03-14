import QtQuick 2.12
import Common 1.0
import QtQuick.Controls 2.12 as Qml
import Components 1.0

Qml.CheckBox {
    id: checkBox

    /*
     * Properties
     */
    property alias richText: richText

    /*
     * Components
     */
    width: parent.width
    spacing: 8

    contentItem: RichText {
        id: richText

        text: checkBox.text
        horizontalAlignment: Text.AlignLeft
        leftPadding: checkBox.indicator.width + checkBox.spacing
        anchors.top: parent.top
        wrapMode: RichText.Wrap
        font.pixelSize: 12
    }

    indicator: Rectangle {
        id: checkBoxOutRect

        implicitWidth: 16
        implicitHeight: 16
        radius: 4
        border.color: getFrameColor()
        border.width: 2
        color: "transparent"
        anchors {
            left: checkBox.left
            top: checkBox.top
            leftMargin: 5
            topMargin: 3
        }

        Rectangle {
            id: inside

            visible: checkBox.checked || checkBox.down
            color: getBackgroundColor()
            radius: 1
            anchors {
                fill: checkBoxOutRect
                margins: checkBoxOutRect.border.width
            }

            SvgImage {
                id: image
                source: "images/check.svg"
                color: getIconColor()
                anchors.centerIn: parent
                sourceSize: Qt.size(8, 8)
            }

        } // Rectangle -> inside

    } // Rectangle -> checkBoxOutRect

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed:  mouse.accepted = false
        cursorShape: Qt.PointingHandCursor
    }

    /*
     * Functions
     */
    function getFrameColor() {
        if(checkBox.down) {
            return Styles.lightTheme ? "#535B65" : "#BDC0C4"; // Pressed
        } else if(checkBox.hovered) {
            return Styles.lightTheme ? "#39424E" : "#A3A6AD"; //hover
        } else if(!checkBox.enabled) {
            return Styles.lightTheme ? "#1A000000" : "#1AFFFFFF"; // disabled
        } else {
            return Styles.lightTheme ? "#04101E" : "#F4F4F5"; //normal
        }
    }

    function getBackgroundColor() {
        if(checkBox.down) {
            if(checkBox.checked) {
                return Styles.lightTheme ? "#535B65" : "#BDC0C4"; // Pressed
            } else {
                return Styles.alternateBackgroundColor
            }
        } else if(checkBox.hovered) {
            return Styles.lightTheme ? "#39424E" : "#A3A6AD"; //hover
        } else if(!checkBox.enabled) {
            return Styles.lightTheme ? "#1A000000" : "#1AFFFFFF"; // disabled
        } else {
            return Styles.lightTheme ? "#04101E" : "#F4F4F5"; //normal
        }
    }

    function getIconColor() {
        if(checkBox.down && !checkBox.checked) {
            return Styles.lightTheme ? "#535B65" : "#BDC0C4"; // Pressed
        } else {
            return Styles.lightTheme ? "#FAFAFA" : "#04101E"
        }
    }
}
