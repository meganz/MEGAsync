import QtQml 2.12

// Local
import Onboarding 1.0

ConfirmEmailPageForm {
    id: confirmEmailPage

    changeEmailLinkText.onLinkActivated: {
        registerFlow.state = changeConfirmEmail;
    }

    cancelAccount.onClicked: {
        loginController.cancelCreateAccount();
    }

    email: loginController.email

    Connections {
        target: loginController

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
