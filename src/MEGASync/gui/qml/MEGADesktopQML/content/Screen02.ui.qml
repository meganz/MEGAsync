

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12
import Styles 1.0
import QtQuick.Layouts 1.11

Rectangle {
    width: Styles.width
    height: Styles.height
    color: Styles.backgroundColor
    property alias roundButton: roundButton
    property string buttonText: qsTr("SuperButtonText")

    ColumnLayout {
        id: columnLayout
        anchors.fill: parent

        Text {
            id: text1
            text: qsTr("Super cool dialog with buttons")
            font.pixelSize: 12
            Layout.fillWidth: true
            horizontalAlignment: Qt.AlignHCenter
        }

        RoundButton {
            id: roundButton
            text: buttonText
            Layout.alignment: Qt.AlignHCenter
        }
    }
}



