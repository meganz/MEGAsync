// QML common
import Components 1.0
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

RegisterPageForm {

    nextButton.onClicked: {
        var error = false;

        if(firstName.text.length === 0 || lastName.text.length === 0) {
            error = true;
            firstLastNameDescription.visible = true;
            firstName.hint.type = HintText.Type.Error;
            lastName.hint.type = HintText.Type.Error;
        } else {
            firstLastNameDescription.visible = false;
            firstName.hint.type = HintText.Type.None;
            lastName.hint.type = HintText.Type.None;
        }

        if(!email.valid()) {
            error = true;
            email.hint.description = OnboardingStrings.errorValidEmail;
            email.hint.type = HintText.Type.Error;
        } else {
            email.hint.type = HintText.Type.None;
        }

        if(password.text.length === 0) {
            error = true;
            password.hint.type = HintText.Type.Error;
        }

        if(confirmPassword.text.length === 0) {
            error = true;
            confirmPassword.hint.description = OnboardingStrings.errorConfirmPassword;
            confirmPassword.hint.type = HintText.Type.Error;
        } else if(password.text.length !== 0 && password.text !== confirmPassword.text) {
            error = true;
            password.hint.type = HintText.Type.Error;
            confirmPassword.hint.description = OnboardingStrings.errorPasswordsMatch;
            confirmPassword.hint.type = HintText.Type.Error;
        } else {
            password.hint.type = HintText.Type.None;
            confirmPassword.hint.type = HintText.Type.None;
        }

        if(error) {
            return;
        }

        var formData = {
            [Onboarding.PASSWORD]: password.text,
            [Onboarding.EMAIL]: email.text,
            [Onboarding.FIRST_NAME]: firstName.text,
            [Onboarding.LAST_NAME]: lastName.text
        }

        Onboarding.onRegisterClicked(formData);
    }

    loginButton.onClicked: {
        registerFlow.state = login;
    }
}
