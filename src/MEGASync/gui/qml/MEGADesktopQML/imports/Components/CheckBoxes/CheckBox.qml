import QtQuick 2.12
import Styles 1.0
import QtGraphicalEffects 1.15
import QtQuick.Controls 2.12 as Qml
import Components 1.0

Qml.CheckBox {

    function getColor()
    {
        if(checkBox.down)
        {
            return Styles.lightTheme ? "#535B65" : "#BDC0C4"; // Pressed
        }
        else if(checkBox.hovered)
        {
            return Styles.lightTheme ? "#39424E" : "#A3A6AD"; //hover
        }
        else if(!checkBox.enabled)
        {
            return Styles.lightTheme ? "#1A000000" : "#1AFFFFFF"; // disabled
        }
        else
        {
            return Styles.lightTheme ? "#04101E" : "#F4F4F5"; //normal
        }
    }

    id: checkBox
    width: parent.width
    spacing: 8
    contentItem: Qml.Label {
        text: checkBox.text
        font: checkBox.font
        horizontalAlignment: Text.AlignLeft
        //verticalAlignment: Text.AlignTop
        leftPadding: checkBox.indicator.width + checkBox.spacing
        anchors.top: parent.top
        wrapMode: Qml.Label.Wrap
    }

    indicator: Rectangle {
        id: checkBoxOutRect
        implicitWidth: 16
        implicitHeight: 16
        radius: 4
        border.color: getColor()
        border.width: 2
        color: "transparent"
        anchors{
            left: checkBox.left
            top: checkBox.top
            leftMargin: 5
            topMargin: 3
        } /*TERMINAR CON LOS COLORES DE LOS CHECKBOXES AL CAMBIAR EL ESTADO DE UNCHECKED A CHECKED!!!*/

        Rectangle {
            anchors{
                fill: checkBoxOutRect
                margins: checkBoxOutRect.border.width
            }
            visible: checkBox.checked
            color: getColor()
            radius: 1
            SvgImage{
                id: image
                source: "images/check.svg"
                color: Styles.lightTheme ? "#FAFAFA" : "#04101E"
                anchors.centerIn: parent
            }
        }
    }
}
