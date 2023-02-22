

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.12
import Styles 1.0

Text {
    property string url: url
    id: control
    color: Styles.textColor
    textFormat: Text.RichText
    Component.onCompleted: {
        control.text = control.text.replace("[b]","<b>")
        control.text = control.text.replace("[/b]","</b>")
        control.text = control.text.replace("[a]","<b>
                                            <a style=\"text-decoration:none\"
                                            style=\"color:#DD1405;\" href=\"" + url + "\">")
        control.text = control.text.replace("[/a]","</a></b></font>")
    }
    onLinkActivated: Qt.openUrlExternally(url)
}

