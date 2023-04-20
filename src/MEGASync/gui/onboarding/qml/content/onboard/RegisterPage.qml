// QML common
import Components 1.0
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

RegisterPageForm {

    nextButton.onClicked: {
        var error = firstName.text.length !== 0 && lastName.text.length !== 0;
        firstLastNameDescription.visible = !error;
        firstName.showType = !error;
        lastName.showType = !error;

        var valid = email.valid();
        if(!valid) {
            error = true;
            email.hint.text = OnboardingStrings.errorValidEmail;
        }
        email.showType = !valid;
        email.hint.visible = !valid;

        valid = password.text.length !== 0;
        if(!valid) {
            error = true;
        }
        password.showType = !valid;

        if(confirmPassword.text.length === 0) {
            error = true;
            confirmPassword.showType = true;
            confirmPassword.hint.visible = true;
            confirmPassword.hint.text = OnboardingStrings.errorConfirmPassword;
        } else if(password.text !== confirmPassword.text) {
            error = true;
            confirmPassword.showType = true;
            confirmPassword.hint.visible = true;
            confirmPassword.hint.text = OnboardingStrings.errorPasswordsMatch;
        } else {
            confirmPassword.showType = false;
            confirmPassword.hint.visible = false;
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
