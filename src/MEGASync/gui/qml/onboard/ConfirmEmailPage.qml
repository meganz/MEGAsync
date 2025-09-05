import QtQuick 2.15

import LoginController 1.0

import ServiceUrls 1.0

ConfirmEmailPageForm {
    id: root

    changeEmailLinkText.onLinkActivated: {
        loginControllerAccess.state = LoginController.CHANGING_REGISTER_EMAIL;
    }

    Connections {
        target: loginControllerAccess

        function onEmailConfirmed() {
            window.raise();
        }
    }

    Connections {
        target: window

        function onInitializePageFocus() {
            bodyText2.forceActiveFocus();
        }
    }
}
