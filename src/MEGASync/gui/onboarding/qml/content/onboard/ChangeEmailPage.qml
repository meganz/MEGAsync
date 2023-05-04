import QtQml 2.12

// Local
import Onboarding 1.0
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
        target: Onboarding

        onChangeRegistrationEmailFinished: (success) => {
            if(success) {
                registerFlow.state = confirmEmail;
            } else {
                emailTextField.showType = true;
                emailTextField.hint.text = OnboardingStrings.errorEmailAlreadyExist;
                emailTextField.hint.visible = true;
            }
        }
    }
}
