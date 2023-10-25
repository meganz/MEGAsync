import QtQml 2.15

// QML common
import Common 1.0

// Local
import Onboard 1.0
import LoginController 1.0

RegisterPageForm {
    id: registerPage

    nextButton.onClicked: {

        if(registerContent.error()) {
            return;
        }

        loginControllerAccess.createAccount(registerContent.email.text,
                                      registerContent.password.text,
                                      registerContent.firstName.text,
                                      registerContent.lastName.text);
    }

    loginButton.onClicked: {
        loginControllerAccess.state = LoginController.LOGGED_OUT;
    }
}
