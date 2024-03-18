import QtQuick 2.15

import common 1.0

import components.textFields 1.0

TextField {
    id: root

    property bool cleanWhenError: true

    rightIconVisible: false
    rightIconSource: Images.eye

    textField {
        echoMode: TextInput.Password
        onTextChanged: {
            rightIconVisible = textField.focus && textField.text.length > 0;
        }
    }

    rightIconMouseArea.onClicked: {
        if(textField.echoMode === TextInput.Password) {
            textField.echoMode = TextInput.Normal;
            rightIconSource = Images.eyeOff;
        }
        else if(textField.echoMode === TextInput.Normal) {
            textField.echoMode = TextInput.Password;
            rightIconSource = Images.eye;
        }
    }

    onErrorChanged: {
        if(error && cleanWhenError) {
            textField.text = "";
        }
    }
}
