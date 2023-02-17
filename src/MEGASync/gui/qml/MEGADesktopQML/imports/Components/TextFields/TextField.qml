

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml

Qml.TextField {
    id: control
    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 40
        color: control.enabled ? "#FFFFFF" : "#353637"
        border.color: control.focus ? palette.highlight : "#D8D9DB"
        border.width: 2
        radius: 8
    }
}


/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

