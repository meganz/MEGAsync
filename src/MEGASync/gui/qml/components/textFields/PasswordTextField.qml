// System
import QtQuick 2.15

// Local
import Components.TextFields 1.0 as MegaTextFields

MegaTextFields.TextField {

    property bool cleanWhenError: true
    rightIconVisible: false
    rightIconSource: "images/eye.svg"

    textField {
        echoMode: TextInput.Password
        onTextChanged: {
            rightIconVisible = textField.focus && textField.text.length > 0;
        }
    }

    rightIconMouseArea.onClicked: {
        if(textField.echoMode === TextInput.Password) {
            textField.echoMode = TextInput.Normal;
            rightIconSource = "images/eye-off.svg";
        } else if(textField.echoMode === TextInput.Normal) {
            textField.echoMode = TextInput.Password;
            rightIconSource = "images/eye.svg";
        }
    }

    onErrorChanged: {
        if(error && cleanWhenError) {
            textField.text = "";
        }
    }
}
