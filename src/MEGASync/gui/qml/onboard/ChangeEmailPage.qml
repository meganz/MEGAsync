import QtQuick 2.15

import LoginController 1.0

ChangeEmailPageForm {
    id: root

    emailTextField.text: loginControllerAccess.email

    cancelButton.onClicked: {
        loginControllerAccess.state = LoginController.WAITING_EMAIL_CONFIRMATION;
    }

    resendButton.onClicked: {
        var valid = emailTextField.valid();
        emailTextField.error = !valid;
        emailTextField.hint.visible = !valid;
        if(!valid) {
            emailTextField.hint.text = OnboardingStrings.errorValidEmail;
            return;
        }

        loginControllerAccess.changeRegistrationEmail(emailTextField.text);
    }

    Connections {
        target: loginControllerAccess

        function onChangeRegistrationEmailFinished(success, errorMsg) {
            if(success) {
                loginControllerAccess.state = LoginController.WAITING_EMAIL_CONFIRMATION;
            }
            else {
                emailTextField.error = true;
                emailTextField.hint.text = errorMsg;
                emailTextField.hint.visible = true;
            }
        }
    }

    Connections {
        target: onboardingWindow

        function onInitializePageFocus() {
            emailTextField.setFocus(true);
        }
    }
}
