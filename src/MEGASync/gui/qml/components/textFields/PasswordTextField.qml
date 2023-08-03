// System
import QtQuick 2.12

// Local
import Components.TextFields 1.0 as MegaTextFields

MegaTextFields.TextField {

    textField.echoMode: TextInput.Password

    textField.onFocusChanged: {
        if(textField.focus) {
            rightIcon.visible = true;
        }
    }

    rightIcon.visible: false
    rightIcon.source: "images/eye.svg"
    rightIconMouseArea.onClicked: {
        if(textField.echoMode === TextInput.Password) {
            textField.echoMode = TextInput.Normal;
            rightIcon.source = "images/eye-off.svg";
        } else if(textField.echoMode === TextInput.Normal) {
            textField.echoMode = TextInput.Password;
            rightIcon.source = "images/eye.svg";
        }
    }

    textField.onTextChanged: {
        error = false;
        hint.visible = false;
    }

}
