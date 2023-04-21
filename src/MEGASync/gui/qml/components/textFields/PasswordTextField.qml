// System
import QtQuick 2.12

// Local
import Components 1.0 as Custom
import Common 1.0

Custom.TextField {

    textField.echoMode: TextInput.Password
    textField.onTextChanged: {
        rightIcon.visible = text.length != 0;
    }

    rightIcon.visible: false
    rightIcon.source: "images/eye.svg"
    rightIconMouseArea.onClicked: {
        textField.focus = true;
        if(textField.echoMode === TextInput.Password) {
            textField.echoMode = TextInput.Normal;
            rightIcon.source = "images/eye-off.svg";
        } else if(textField.echoMode === TextInput.Normal) {
            textField.echoMode = TextInput.Password;
            rightIcon.source = "images/eye.svg";
        }
    }

    type: Custom.TextField.Type.Error

}
