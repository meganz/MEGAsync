// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// Local
import Components 1.0 as Custom
import Common 1.0

// C++
import QmlClipboard 1.0

ColumnLayout {
    id: root

    function pastePin() {
        const regex = RegexExpressions.digit2FA;
        var pin = QmlClipboard.text().slice(0, 6);
        if (!regex.test(pin)) {
            console.warn("Invalid 2FA pin format pasted");
            return;
        }

        digit1.textField.text = pin.charAt(0);
        digit2.textField.text = pin.charAt(1);
        digit3.textField.text = pin.charAt(2);
        digit4.textField.text = pin.charAt(3);
        digit5.textField.text = pin.charAt(4);
        digit6.textField.text = pin.charAt(5);
    }

    property string key: digit1.textField.text + digit2.textField.text + digit3.textField.text
                         + digit4.textField.text + digit5.textField.text + digit6.textField.text
    property bool hasError: false

    spacing: 12
    width: parent.width

    RowLayout {
        id: mainLayout

        function error() {
            return hasError ? TextField.DescriptionType.Error : TextField.DescriptionType.None;
        }

        Layout.preferredWidth: root.width
        Layout.preferredHeight: 72
        spacing: 4

        TwoFADigit {
            id: digit1

            next: digit2
            Layout.preferredHeight: 72
            hint.type: mainLayout.error()
        }

        TwoFADigit {
            id: digit2

            Layout.preferredHeight: 72
            next: digit3
            previous: digit1
            hint.type: mainLayout.error()
        }

        TwoFADigit {
            id: digit3

            Layout.preferredHeight: 72
            next: digit4
            previous: digit2
            hint.type: mainLayout.error()
        }

        TwoFADigit {
            id: digit4

            Layout.preferredHeight: 72
            next: digit5
            previous: digit3
            hint.type: mainLayout.error()
        }

        TwoFADigit {
            id: digit5

            Layout.preferredHeight: 72
            next: digit6
            previous: digit4
            hint.type: mainLayout.error()
        }

        TwoFADigit {
            id: digit6

            Layout.preferredHeight: 72
            previous: digit5
            hint.type: mainLayout.error()
        }
    }

    Custom.HintText {
        id: hint

        Layout.leftMargin: 4
        title: qsTr("Authentication failed")
        description: qsTr("Please, try again.")
        type: hasError ? Custom.HintText.Type.AuthenticationError : Custom.HintText.Type.None
    }

    Shortcut {
        sequence: "Ctrl+V"
        onActivated: {
            pastePin();
        }
    }
}



