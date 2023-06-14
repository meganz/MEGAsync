// System
import QtQuick 2.12
import QtQuick.Controls 2.12

// Local
import Components.TextFields 1.0 as MegaTextFields
import Common 1.0

MegaTextFields.TextField {

    property var next
    property var previous

    readonly property int widthWidthFocus: 66
    readonly property int heightWithFocus: 78

    height: heightWithFocus
    width: widthWidthFocus

    textField.validator: RegExpValidator { regExp: RegexExpressions.digit2FA }
    textField.height: height + 6 // add the focus border size (3 up + 3 down)
    textField.font.pixelSize: 48
    textField.font.weight: Font.DemiBold
    textField.leftPadding: 19
    textField.bottomPadding: 10

    textField.onTextChanged: {
        if(textField.text.length !== 0 && next !== undefined) {
            next.textField.focus = true;
        } else {
            textField.focus = false;
        }
    }

    textField.onFocusChanged: {
        if(textField.focus) {
            textField.select(0, 1);
        }
    }

    onBackPressed: {
        if(previous !== undefined) {
            previous.textField.focus = true;
        }
    }

    onPastePressed: {
        pastePin();
    }

}
