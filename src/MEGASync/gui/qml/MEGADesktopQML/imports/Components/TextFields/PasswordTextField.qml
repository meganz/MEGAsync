

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12
import Components 1.0 as Custom
import Styles 1.0

Custom.TextField {
    id: control
    textField.echoMode: TextInput.Password
    textField.onTextChanged: {
        if(textField.text.length === 0)
        {
            button.iconColor = Styles.lightTheme ?  "#C1C2C4" : "#797C80"
        }
        else
        {
            button.iconColor = Styles.lightTheme ? "#616366" : "#A9ABAD"
        }
    }
    RoundButton {
        property url iconSource: "images/eye.svg"
        property string iconColor: "#C1C2C4"

        id: button
        anchors.right: control.right
        palette.button: "transparent"
        icon.source: iconSource
        icon.color: iconColor
        icon.height: 16
        icon.width: 16
        height: control.height
        width: control.height
        MouseArea{
            id: mouseArea
            anchors.fill: parent
            onClicked: {
                if(textField.echoMode === TextInput.Password)
                {
                    textField.echoMode = TextInput.Normal
                    button.iconSource = "images/eye-off.svg"
                }
                else if(textField.echoMode === TextInput.Normal)
                {
                    textField.echoMode = TextInput.Password
                    button.iconSource = "images/eye.svg"
                }
            }
            cursorShape: Qt.PointingHandCursor
        }
    }
}


/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

