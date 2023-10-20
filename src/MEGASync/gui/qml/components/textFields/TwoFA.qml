// System
import QtQuick 2.15
import QtQuick.Layouts 1.15

// Local
import Components.Texts 1.0 as MegaTexts
import Components.TextFields 1.0 as MegaTextFields
import Common 1.0
import Onboard 1.0

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

    signal allDigitsFilled

    spacing: 15
    Layout.leftMargin: -digit1.sizes.focusBorderWidth

    onKeyChanged: {
        if(key.length === 6) {
            allDigitsFilled();
        }
    }

    RowLayout {
        id: mainLayout

        Layout.preferredHeight: digit1.heightWithFocus
        spacing: 2

        TwoFADigit {
            id: digit1

            next: digit2
        }

        TwoFADigit {
            id: digit2

            next: digit3
            previous: digit1
        }

        TwoFADigit {
            id: digit3

            next: digit4
            previous: digit2
        }

        TwoFADigit {
            id: digit4

            next: digit5
            previous: digit3
        }

        TwoFADigit {
            id: digit5

            next: digit6
            previous: digit4
        }

        TwoFADigit {
            id: digit6

            previous: digit5
        }
    }

    MegaTexts.NotificationText {
        id: notification

        visible: hasError
        Layout.leftMargin: 3
        title: OnboardingStrings.authFailed
        text: OnboardingStrings.tryAgain
        Layout.preferredWidth: root.width - 4
        Layout.preferredHeight: notification.height
        attributes.type: MegaTexts.NotificationInfo.Type.Error
        attributes.icon.source: Images.lock
        time: 2000

        onVisibilityTimerFinished: {
            hasError = false;
            digit1.textField.text = "";
            digit2.textField.text = "";
            digit3.textField.text = "";
            digit4.textField.text = "";
            digit5.textField.text = "";
            digit6.textField.text = "";
            digit1.textField.forceActiveFocus();
        }
    }

    Shortcut {
        sequence: "Ctrl+V"
        onActivated: {
            pastePin();
        }
    }

}



