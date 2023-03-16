

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Components 1.0 as Custom

Item {
    id: root
    property string key2fa: digit1.textField.text + digit2.textField.text + digit3.textField.text + digit4.textField.text + digit5.textField.text + digit6.textField.text
    property int fHeight: 72
    property int fWidth: 60
    property bool hasError: false

    Custom.TextField {
        id: digit1
        textField.validator: RegExpValidator{regExp: /^[0-9]{1}$/}
        fieldHeight: fHeight
        width: fWidth
        error: hasError
        font.pixelSize: digit1.height - 30
        textField.onTextChanged: {
            if(textField.text.length !== 0)
            {
                digit2.textField.focus = true;
            }
        }
        textField.onFocusChanged:
        {
            if(textField.focus)
            {
                textField.select(0,1);
            }
        }

    }
    Custom.TextField {
        id: digit2
        textField.validator: RegExpValidator{regExp: /^[0-9]{1}$/}
        fieldHeight: fHeight
        width: fWidth
        error: hasError
        anchors.left: digit1.right
        font.pixelSize: digit1.height - 30
        textField.onTextChanged: {
            if(textField.text.length !== 0)
            {
                digit3.textField.focus = true;
            }
        }
        textField.onFocusChanged:
        {
            if(textField.focus)
            {
                textField.select(0,1);
            }
        }
        onBackPressed:
        {
            digit1.textField.focus = true;
        }
    }
    Custom.TextField {
        id: digit3
        textField.validator: RegExpValidator{regExp: /^[0-9]{1}$/}
        fieldHeight: fHeight
        width: fWidth
        error: hasError
        anchors.left: digit2.right
        font.pixelSize: digit1.height - 30
        textField.onTextChanged: {
            if(textField.text.length !== 0)
            {
                digit4.textField.focus = true;
            }
        }
        textField.onFocusChanged:
        {
            if(textField.focus)
            {
                textField.select(0,1);
            }
        }
        onBackPressed:
        {
            digit2.textField.focus = true;
        }
    }
    Custom.TextField {
        id: digit4
        textField.validator: RegExpValidator{regExp: /^[0-9]{1}$/}
        fieldHeight: fHeight
        width: fWidth
        error: hasError
        anchors.left: digit3.right
        font.pixelSize: digit1.height - 30
        textField.onTextChanged: {
            if(textField.text.length !== 0)
            {
                digit5.textField.focus = true;
            }
        }
        textField.onFocusChanged:
        {
            if(textField.focus)
            {
                textField.select(0,1);
            }
        }
        onBackPressed:
        {
            digit3.textField.focus = true;
        }
    }
    Custom.TextField {
        id: digit5
        textField.validator: RegExpValidator{regExp: /^[0-9]{1}$/}
        fieldHeight: fHeight
        width: fWidth
        error: hasError
        anchors.left: digit4.right
        font.pixelSize: digit1.height - 30
        textField.onTextChanged: {
            if(textField.text.length !== 0)
            {
                digit6.textField.focus = true;
            }
        }
        textField.onFocusChanged:
        {
            if(textField.focus)
            {
                textField.select(0,1);
            }
        }
        onBackPressed:
        {
            digit4.textField.focus = true;
        }
    }
    Custom.TextField {
        id: digit6
        textField.validator: RegExpValidator{regExp: /^[0-9]{1}$/}
        fieldHeight: fHeight
        width: fWidth
        error: hasError
        anchors.left: digit5.right
        font.pixelSize: digit1.height - 30
        textField.onFocusChanged:
        {
            if(textField.focus)
            {
                textField.select(0,1);
            }
        }
        onBackPressed:
        {
            digit5.textField.focus = true;
        }
    }
    Rectangle{
        visible: hasError
        color: "#FF0000"
        height: 50
        anchors{
            top: digit1.bottom
            right: digit6.right
            left: digit1.left
            topMargin: 10
        }
        Text {
            id: text
            text: qsTr("Authentication failed")
            x: 10
        }
    }
}



/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

