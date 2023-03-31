import QtQuick 2.12
import QtQuick.Controls 2.12

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
}



