import QtQml 2.12

// Local
import Onboarding 1.0

ConfirmEmailPageForm {
    id: confirmEmailPage

    changeEmailLinkText.onLinkActivated: {
        registerFlow.state = changeConfirmEmail
    }

    email: Onboarding.email

    Connections {
        target: Onboarding

        onAccountConfirmed:{
            registerFlow.state = login
        }
    }
}
