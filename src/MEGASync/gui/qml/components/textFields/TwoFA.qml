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

        digit1.text = pin.charAt(0);
        digit2.text = pin.charAt(1);
        digit3.text = pin.charAt(2);
        digit4.text = pin.charAt(3);
        digit5.text = pin.charAt(4);
        digit6.text = pin.charAt(5);
    }

    property string key: digit1.text + digit2.text + digit3.text + digit4.text + digit5.text + digit6.text
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

            error: hasError
            next: digit2
            onPastePressed: {
                pastePin();
            }
        }

        TwoFADigit {
            id: digit2

            error: hasError
            next: digit3
            previous: digit1
            onPastePressed: {
                pastePin();
            }
        }

        TwoFADigit {
            id: digit3

            error: hasError
            next: digit4
            previous: digit2
            onPastePressed: {
                pastePin();
            }
        }

        TwoFADigit {
            id: digit4

            error: hasError
            next: digit5
            previous: digit3
            onPastePressed: {
                pastePin();
            }
        }

        TwoFADigit {
            id: digit5

            error: hasError
            next: digit6
            previous: digit4
            onPastePressed: {
                pastePin();
            }
        }

        TwoFADigit {
            id: digit6

            error: hasError
            previous: digit5
            onPastePressed: {
                pastePin();
            }
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
        type: Constants.MessageType.ERROR
        icon: Images.lock
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
        sequence: StandardKey.Paste
        onActivated: {
            pastePin();
        }
    }

}



