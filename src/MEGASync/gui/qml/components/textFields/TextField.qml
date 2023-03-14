import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import Common 1.0
import QtQuick.Layouts 1.12
import Components 1.0 as Custom

Item {
    id: root

    /*
     * Properties
     */
    property alias textField: textField
    property alias outRect: outRect
    property alias placeholderText: textField.placeholderText
    property alias font: textField.font
    property bool showInformativeText: true
    property bool error: false
    property string informativeTextIcon: ""
    property string informativeText: ""
    property color textColor: Styles.textColor
    property int fieldHeight: 50

    /*
     * Components
     */
    implicitHeight: outRect.height

    function getExternalFocusedRectColor()
    {
        if(Styles.lightTheme)
        {
            return "#BDD9FF"
        }
        else
        {
            return "#2647D0"
        }
    }

    Rectangle
    {
        id: outRect

        border.width: 4
        implicitHeight: fieldHeight
        radius: 12
        border.color: {
            if(textField.focus)
            {
                getExternalFocusedRectColor()
            }
            else
            {
                Styles.backgroundColor
            }
        }

        anchors {
            top: root.top
            right: root.right
            left: root.left
        }

        Qml.TextField {
            id: textField

            selectByMouse: true
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: 4
            color: Styles.lightTheme ? "#303233" : "#F3F4F4"
            placeholderTextColor: Styles.lightTheme ? "#A9ABAD" : "#919397"
            topPadding: 12
            rightPadding: 14
            bottomPadding: 12
            leftPadding: 14
            font.pixelSize: 14
            selectionColor: getExternalFocusedRectColor()

            anchors {
                fill: outRect
                margins:outRect.border.width
            }

            background: Rectangle {
                id: inRect

                implicitWidth: 200
                implicitHeight: 36
                color: Styles.alternateBackgroundColor
                border.color: getBorderColor()
                border.width: 2
                radius: 8
            }

        } // Qml.TextField -> textField

    } // Rectangle -> outRect

    Loader {
        id: loader

        sourceComponent: informativeTextIcon.length !== 0 ? textWithIcon : onlyText
        height: parent.height
        visible: showInformativeText

        anchors {
            top: outRect.bottom
            left: root.left
            leftMargin: textField.leftPadding
            topMargin: 6
            right: root.right
            bottom: root.bottom
        }

        onVisibleChanged: {
            root.implicitHeight = outRect.height + (item ? item.implicitHeight : 0)
        }
    } // Loader -> loader

    Component {
        id: textWithIcon

        RowLayout {
            Layout.leftMargin: textField.leftPadding
            spacing: 8

            Custom.SvgImage {
                id: textIcon

                height: 8
                width: 8
                sourceSize: Qt.size(20, 20)
                source: informativeTextIcon
                color: getTextColor()
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: false
                Layout.topMargin: 4
                Layout.leftMargin: 6
            }

            Custom.RichText {
                id: infoText

                wrapMode: Text.WordWrap
                text: informativeText
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                color: getTextColor()
                Layout.leftMargin: 6
            }
        }
    } // Component -> textWithIcon

    Component {
        id: onlyText

        RowLayout {
            Layout.leftMargin: textField.leftPadding

            Custom.RichText {
                id: infoText

                wrapMode: Text.WordWrap
                text: informativeText
                Layout.leftMargin: 6
                Layout.fillWidth: true
                color: getBorderColor()
            }
        }
    } // Component -> onlyText

    /*
     * Functions
     */
    function getBorderColor() {
        if(error) {
            return Styles.lightTheme ?  "#E31B57" : "#FD6F90";
        } else if(textField.focus) {
            return Styles.lightTheme ? "#04101E" : "#F4F4F5";
        } else {
            return Styles.lightTheme ?  "#D8D9DB" : "#616366";
        }
    }

    function getTextColor() {
        if(error) {
            return Styles.lightTheme ?  "#E31B57" : "#FD6F90";
        } else {
            return Styles.textColor
        }
    }
}






