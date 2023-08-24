// System
import QtQuick 2.12

// Local
import Onboard 1.0

// C++
import LoginController 1.0

ChangeEmailPageForm {

    cancelButton.onClicked: {
        LoginControllerAccess.state = LoginController.WAITING_EMAIL_CONFIRMATION;
    }

    resendButton.onClicked: {
        var valid = emailTextField.valid();
        emailTextField.error = !valid;
        emailTextField.hint.visible = !valid;
        if(!valid) {
            emailTextField.hint.text = OnboardingStrings.errorValidEmail;
            return;
        }

        LoginControllerAccess.changeRegistrationEmail(emailTextField.text);
    }

    emailTextField.text: LoginControllerAccess.email

    Connections {
        target: LoginControllerAccess

        onChangeRegistrationEmailFinished: (success) => {
            if(success) {
                LoginControllerAccess.state = LoginController.WAITING_EMAIL_CONFIRMATION;
            } else {
                emailTextField.error = true;
                emailTextField.hint.text = OnboardingStrings.errorEmailAlreadyExist;
                emailTextField.hint.visible = true;
            }
        }
    }
}
