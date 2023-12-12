import QtQuick 2.15

import LoginController 1.0

RegisterPageForm {
    id: root

    nextButton.onClicked: {
        if(registerContent.error()) {
            return;
        }

        loginControllerAccess.createAccount(registerContent.email.text,
                                            registerContent.password.text,
                                            registerContent.firstName.text,
                                            registerContent.lastName.text);
        onboardingWindow.creatingAccount = true;
    }

    loginButton.onClicked: {
        loginControllerAccess.state = LoginController.LOGGED_OUT;
    }

    Connections {
        target: onboardingWindow

        function onInitializePageFocus() {
            registerContent.firstName.setFocus(true);
        }
    }

}
