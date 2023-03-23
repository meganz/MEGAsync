import QtQuick 2.12
import QtQuick.Controls 2.12

import Onboarding 1.0

TwoFAPageForm {

    cancelButton.onClicked: {
        registerStack.replace(registerPage)
    }

    acceptButton.onClicked: {
        if(key.length === 6) {
            Onboarding.onTwoFACompleted(key);
        }
    }
}
