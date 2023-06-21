import QtQml 2.12

// Local
import Onboarding 1.0

ConfirmEmailPageForm {
    id: confirmEmailPage

    changeEmailLinkText.onLinkActivated: {
        registerFlow.state = changeConfirmEmail;
    }

    email: loginController.email

    Connections {
        target: loginController

        onAccountConfirmed: {
            registerFlow.state = login;
        }
    }
}
