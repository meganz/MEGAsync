import QtQml 2.12

// Local
import Onboard 1.0

ChangeEmailPageForm {

    cancelButton.onClicked: {
        registerFlow.state = confirmEmail;
    }

    resendButton.onClicked: {
        Onboarding.changeRegistrationEmail(emailTextField.text);
    }

    emailTextField.text: Onboarding.email

    Connections {
        target: loginController

        onChangeRegistrationEmailFinished: (success) => {
            if(success) {
                registerFlow.state = confirmEmail;
            } else {
                emailTextField.error = true;
                emailTextField.hint.text = OnboardingStrings.errorEmailAlreadyExist;
                emailTextField.hint.visible = true;
            }
        }
    }
}
