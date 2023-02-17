

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12
import Components 1.0 as Custom

Custom.TextField {
    id: control
    echoMode: TextInput.Password
    RoundButton {
        anchors.right: control.right
        palette.button: "transparent"
        icon.source: "images/eye.png"
        MouseArea{
            id: mouseArea
            anchors.fill: parent
            onClicked: {
                if(control.echoMode == TextInput.Password)
                {
                    control.echoMode = TextInput.Normal
                }
                else if(control.echoMode == TextInput.Normal)
                {
                    control.echoMode = TextInput.Password
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

