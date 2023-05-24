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

        Layout.preferredHeight: 72
        spacing: 8

        TwoFADigit {
            id: digit1

            next: digit2
            Layout.preferredWidth: 60
            Layout.preferredHeight: 72
            hint.visible: hasError
            onTextChanged: { hasError = false }
        }

        TwoFADigit {
            id: digit2

            Layout.preferredWidth: 60
            Layout.preferredHeight: 72
            next: digit3
            previous: digit1
            hint.visible: hasError
            onTextChanged: { hasError = false }
        }

        TwoFADigit {
            id: digit3

            Layout.preferredWidth: 60
            Layout.preferredHeight: 72
            next: digit4
            previous: digit2
            hint.visible: hasError
            onTextChanged: { hasError = false }
        }

        TwoFADigit {
            id: digit4

            Layout.preferredWidth: 60
            Layout.preferredHeight: 72
            next: digit5
            previous: digit3
            hint.visible: hasError
            onTextChanged: { hasError = false }
        }

        TwoFADigit {
            id: digit5

            Layout.preferredWidth: 60
            Layout.preferredHeight: 72
            next: digit6
            previous: digit4
            hint.visible: hasError
            onTextChanged: { hasError = false }
        }

        TwoFADigit {
            id: digit6

            Layout.preferredWidth: 60
            Layout.preferredHeight: 72
            previous: digit5
            onTextChanged: { hasError = false }
        }
    }

    Custom.NotificationText {
        id: notification

        visible: hasError
        Layout.leftMargin: 3
        title: qsTr("Authentication failed")
        notificationText.text: qsTr("Please, try again.")
        Layout.preferredWidth: root.width - 4
        Layout.preferredHeight: notification.height
        type: Custom.NotificationText.Type.AuthenticationError
    }

    Shortcut {
        sequence: "Ctrl+V"
        onActivated: {
            pastePin();
        }
    }
}



