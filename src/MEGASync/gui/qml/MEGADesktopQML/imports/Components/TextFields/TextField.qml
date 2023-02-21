

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import Styles 1.0
import QtQuick.Layouts 1.12

Rectangle
{
    property alias textField: textField
    property alias placeholderText: textField.placeholderText
    property alias font: textField.font

    id: outRect
    border.width: 4
    implicitHeight: 50
    radius: 12
    border.color: {
        if(textField.focus)
        {
            if(Styles.lightTheme)
            {
                "#BDD9FF"
            }
            else
            {
                "#2647D0"
            }
        }
        else
        {
            color: Styles.backgroundColor
        }
    }

    Qml.TextField {
        id: textField
        anchors{
            fill: parent
            margins:outRect.border.width
        }
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

        background: Rectangle {
            id: inRect
            implicitWidth: 200
            implicitHeight: 36
            color: Styles.alternateBackgroundColor
            border.color: {
                if(textField.focus)
                {
                    if(Styles.lightTheme)
                    {
                        "#04101E"
                    }
                    else
                    {
                        "#F4F4F5"
                    }
                }
                else
                {
                    if(Styles.lightTheme)
                    {
                        "#D8D9DB"
                    }
                    else
                    {
                        "#616366"
                    }
                }
            }
            border.width: 2
            radius: 8
        }
    }
}



/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
