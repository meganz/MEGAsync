// System
import QtQuick 2.12

// Local
import Onboarding 1.0

// C++
import LoginController 1.0

ConfirmEmailPageForm {
    id: confirmEmailPage

    changeEmailLinkText.onLinkActivated: {
        registerFlow.state = changeConfirmEmail;
    }

    cancelAccount.onClicked: {
        LoginControllerAccess.cancelCreateAccount();
    }

    email: LoginControllerAccess.email

    Connections {
        target: LoginControllerAccess

        onEmailConfirmed: {
            registerFlow.state = login;
            onboardingWindow.requestActivate();
            onboardingWindow.raise();
        }

        onAccountCreateCancelled: {
            registerFlow.state = login;
        }
    }
}
