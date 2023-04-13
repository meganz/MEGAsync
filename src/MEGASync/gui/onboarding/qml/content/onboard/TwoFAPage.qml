// System
import QtQuick 2.12
import QtQuick.Controls 2.12

// C++
import Onboarding 1.0

TwoFAPageForm {

    signUpButton.onClicked: {
        registerFlow.state = register;
    }

    loginButton.onClicked: {
        Onboarding.onTwoFARequested(twoFAField.key);
    }

    Connections {
        target: Onboarding

        onTwoFAFailed: {
            twoFAField.hasError = true;
        }
    }
}
