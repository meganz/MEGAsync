import QtQuick 2.12
import QtQuick.Controls 2.12
import Components 1.0 as Custom
import Common 1.0

Custom.TextField {
    id: control

    /*
     * Components
     */
    textField.echoMode: TextInput.Password

    textField.onTextChanged: {
        if(textField.text.length === 0) {
            button.iconColor = Styles.lightTheme ?  "#C1C2C4" : "#797C80"
        } else {
            button.iconColor = Styles.lightTheme ? "#616366" : "#A9ABAD"
        }
    }

    RoundButton {
        id: button

        property url iconSource: "images/eye.svg"
        property string iconColor: "#C1C2C4"

        anchors.right: control.right
        anchors.top: control.top
        palette.button: "transparent"
        icon.source: iconSource
        icon.color: iconColor
        icon.height: 16
        icon.width: 16
        height: outRect.height
        width: outRect.height

        MouseArea {
            id: mouseArea

            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                if(textField.echoMode === TextInput.Password) {
                    textField.echoMode = TextInput.Normal
                    button.iconSource = "images/eye-off.svg"
                } else if(textField.echoMode === TextInput.Normal) {
                    textField.echoMode = TextInput.Password
                    button.iconSource = "images/eye.svg"
                }
            }

        } // MouseArea -> mouseArea

    } // RoundButton -> button

} // Custom.TextField -> control
