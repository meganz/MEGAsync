import QtQml 2.12

// QML common
import Components 1.0
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

RegisterPageForm {
    id: registerPage

    nextButton.onClicked: {
        var error = firstName.text.length === 0 && lastName.text.length === 0;
        firstLastNameDescription.visible = error;
        firstName.showType = error;
        lastName.showType = error;

        console.log(error)

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

        registerPage.enabled = false;

        var formData = {
            [Onboarding.RegisterForm.PASSWORD]: password.text,
            [Onboarding.RegisterForm.EMAIL]: email.text,
            [Onboarding.RegisterForm.FIRST_NAME]: firstName.text,
            [Onboarding.RegisterForm.LAST_NAME]: lastName.text
        }

        Onboarding.onRegisterClicked(formData);
    }

    loginButton.onClicked: {
        registerFlow.state = login;
    }

    Connections {
        target: Onboarding

        onRegisterFinished: {
            registerPage.enabled = true;

            if(apiOk)
            {
                password.text = "";
                confirmPassword.text = "";
                firstName.text = "";
                lastName.text = "";
                email.text = "";
                termsCheckBox.checked = false;
                dataLossCheckBox.checked = false;
                registerFlow.state = confirmEmail; //SET HERE CONFIRM ACCOUNT BY EMAIL PAGE??
            }
            else
            {
                email.showType = true;
                email.hint.text = OnboardingStrings.errorEmailAlreadyExist;
                email.hint.visible = true;
            }
        }
    }
}
