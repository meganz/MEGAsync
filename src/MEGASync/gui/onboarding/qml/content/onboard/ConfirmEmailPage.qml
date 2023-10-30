// System
import QtQuick 2.15
import QtQuick.Window 2.15

// Local
import Onboarding 1.0

// C++
import LoginController 1.0

ConfirmEmailPageForm {
    id: confirmEmailPage

    changeEmailLinkText.onLinkActivated: {
        loginControllerAccess.state = LoginController.CHANGING_REGISTER_EMAIL;
    }

    Connections {
        target: loginControllerAccess

        function onEmailConfirmed() {
            onboardingWindow.raise();
        }
    }
}
