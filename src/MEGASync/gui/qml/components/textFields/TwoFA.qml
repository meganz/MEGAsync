import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Components 1.0 as Custom
import Common 1.0

ColumnLayout {
    id: root

    property string key2fa: digit1.textField.text + digit2.textField.text + digit3.textField.text
                            + digit4.textField.text + digit5.textField.text + digit6.textField.text
    property bool hasError: false

    spacing: 12
    width: parent.width

    RowLayout {

        Layout.preferredWidth: root.width
        spacing: 4

        TwoFADigit {
            id: digit1

            next: digit2
            error: hasError
        }

        TwoFADigit {
            id: digit2

            next: digit3
            previous: digit1
            error: hasError
        }

        TwoFADigit {
            id: digit3

            next: digit4
            previous: digit2
            error: hasError
        }

        TwoFADigit {
            id: digit4

            next: digit5
            previous: digit3
            error: hasError
        }

        TwoFADigit {
            id: digit5

            next: digit6
            previous: digit4
            error: hasError
        }

        TwoFADigit {
            id: digit6

            previous: digit5
            error: hasError
        }
    }

    Rectangle {
        visible: hasError
        color: Styles.textError
        Layout.preferredHeight: 50

        Text {
            id: text
            text: qsTr("Authentication failed")
            x: 10
        }
    }
}



