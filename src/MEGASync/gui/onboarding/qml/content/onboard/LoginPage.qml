import QtQuick 2.12

import Onboarding 1.0

LoginPageForm {

    forgotPasswordHyperlinkArea.onClicked: {
        Onboarding.onForgotPasswordClicked();
    }

    Keys.onEnterPressed: {
        loginButton.clicked();
    }

    Keys.onReturnPressed: {
        loginButton.clicked();
    }

    loginButton.onClicked: {
        if(email.length !== 0 && password.length !== 0) {
            Onboarding.onLoginClicked({[Onboarding.OnboardEnum.EMAIL]: email,
                                       [Onboarding.OnboardEnum.PASSWORD]: password})
        }
    }

}



