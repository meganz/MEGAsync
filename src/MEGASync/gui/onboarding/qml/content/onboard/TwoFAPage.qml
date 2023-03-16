/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick 2.12
import QtQuick.Controls 2.12

TwoFAPageForm {
    cancelButton.onClicked: {
        registerStack.replace(registerPage)
    }
    acceptButton.onClicked: {
        if(key.length === 6)
        {
            Onboarding.onTwoFACompleted(key);
        }
    }
}




/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
