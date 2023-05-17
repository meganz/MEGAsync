import QtQml 2.12

// QML common
import Components 1.0 as Custom
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

RegisterPageForm {
    id: registerPage

    nextButton.onClicked: {
        var error = firstName.text.length === 0 || lastName.text.length === 0;
        firstLastNameHint.visible = error;
        firstName.showType = error;
        lastName.showType = error;

        var valid = email.valid();
        if(!valid) {
            error = true;
            email.hintText = OnboardingStrings.errorValidEmail;
        }
        email.showType = !valid;
        email.hintVisible = !valid;

        valid = password.text.length !== 0;
        if(!valid) {
            error = true;
            password.hintType = Custom.HintText.Type.Error;
            password.type = Custom.TextField.Type.Error;
            password.showType = true;
        }
        password.showType = !valid;

        if(confirmPassword.text.length === 0) {
            error = true;
            confirmPassword.showType = true;
            confirmPassword.hintVisible = true;
            confirmPassword.hintText = OnboardingStrings.errorConfirmPassword;
            confirmPassword.type = Custom.TextField.Type.Error;
            confirmPassword.hintType = Custom.HintText.Type.Error;
        } else if(password.text !== confirmPassword.text) {
            error = true;
            confirmPassword.showType = true;
            confirmPassword.hintVisible = true;
            confirmPassword.type = Custom.TextField.Type.Error;
            confirmPassword.hintText = OnboardingStrings.errorPasswordsMatch;
            confirmPassword.hintType = Custom.HintText.Type.Error;
            password.hintVisible = false;
            password.type = Custom.TextField.Type.Error;
            password.showType = true;
        } else {
            confirmPassword.showType = false;
            confirmPassword.hintVisible = false;
        }

        if(error) {
            return;
        }

        nextButton.icons.busyIndicatorVisible = true;
        state = signUpStatus;

        var formData = {
            [Onboarding.RegisterForm.PASSWORD]: password.text,
            [Onboarding.RegisterForm.EMAIL]: email.text,
            [Onboarding.RegisterForm.FIRST_NAME]: firstName.text,
            [Onboarding.RegisterForm.LAST_NAME]: lastName.text
        }

        Onboarding.onRegisterClicked(formData);
    }

    nextButton.progress.onAnimationFinished: {
        if(completed){

            password.text = "";
            confirmPassword.text = "";
            firstName.text = "";
            lastName.text = "";
            email.text = "";
            termsCheckBox.checked = false;
            dataLossCheckBox.checked = false;
            state = normalStatus;
            nextButton.icons.busyIndicatorVisible = false;
            registerFlow.state = confirmEmail;
        }
    }

    loginButton.onClicked: {
        registerFlow.state = login;
    }

    Connections {
        target: Onboarding

        onRegisterFinished: (success) => {
                                if(success) {
                                    registerFlow.state = confirmEmail;
                                    password.text = "";
                                    confirmPassword.text = "";
                                    firstName.text = "";
                                    lastName.text = "";
                                    email.text = "";
                                    termsCheckBox.checked = false;
                                    dataLossCheckBox.checked = false;
                                } else {
                                    nextButton.progressValue = 0;
                                    email.showType = true;
                                    email.hintText = OnboardingStrings.errorEmailAlreadyExist;
                                    email.hintVisible = true;
                                }
                                state = normalStatus;
                                nextButton.icons.busyIndicatorVisible = false;
                            }
    }
}
