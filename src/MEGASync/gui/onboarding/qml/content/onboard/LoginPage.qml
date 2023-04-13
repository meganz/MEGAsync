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
            password.hint.description = OnboardingStrings.errorEmptyPassword;
        } else {
            password.hint.type = HintText.Type.None;
        }

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



