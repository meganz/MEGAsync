// System
import QtQuick 2.12

// QML common
import Components 1.0

// Local
import Onboarding 1.0

// C++
import Onboard 1.0

LoginPageForm {

    Keys.onEnterPressed: {
        loginButton.clicked();
    }

    Keys.onReturnPressed: {
        loginButton.clicked();
    }

    loginButton.onClicked: {
        var error = false;

        var valid = email.valid();
        if(!valid) {
            error = true;
            email.hint.text = OnboardingStrings.errorValidEmail;
        }
        email.showType = !valid;
        email.hint.visible = !valid;

        valid = (password.text.length !== 0);
        if(!valid) {
            error = true;
            password.hint.text = OnboardingStrings.errorEmptyPassword;
        }
        password.showType = !valid;
        password.hint.visible = !valid;

        if(error) {
            return;
        }

        Onboarding.onLoginClicked({ [Onboarding.OnboardEnum.EMAIL]: email.text,
                                    [Onboarding.OnboardEnum.PASSWORD]: password.text })
    }

    signUpButton.onClicked: {
        registerFlow.state = register;
    }
}



