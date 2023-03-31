import QtQuick 2.12
import Onboarding 1.0

RegisterFlowForm {
    id: registerFlow

    state: login

    loginPage.loginButton.onClicked: {
        if(loginPage.email.length !== 0 && loginPage.password.length !== 0) {
            Onboarding.onLoginClicked({[Onboarding.OnboardEnum.EMAIL]: loginPage.email,
                                       [Onboarding.OnboardEnum.PASSWORD]: loginPage.password})
        }
    }

    Connections{
        target: Onboarding

        onTwoFARequired: {
            registerFlow.state = twoFA;
        }
    }
}
