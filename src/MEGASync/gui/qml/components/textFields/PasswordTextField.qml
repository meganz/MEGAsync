// System
import QtQuick 2.12

// Local
import Components.TextFields 1.0 as MegaTextFields

MegaTextFields.TextField {

    property bool cleanWhenError: true

    textField {
        echoMode: TextInput.Password
        onTextChanged: {
            rightIcon.visible = textField.focus && textField.text.length > 0;
        }
    }

    rightIcon {
        visible: false
        source: "images/eye.svg"
    }

    rightIconMouseArea.onClicked: {
        if(textField.echoMode === TextInput.Password) {
            textField.echoMode = TextInput.Normal;
            rightIcon.source = "images/eye-off.svg";
        } else if(textField.echoMode === TextInput.Normal) {
            textField.echoMode = TextInput.Password;
            rightIcon.source = "images/eye.svg";
        }
    }

    onErrorChanged: {
        if(error && cleanWhenError) {
            textField.text = "";
        }
    }

}
