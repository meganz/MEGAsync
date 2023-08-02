import QtQuick 2.0

import ApiEnums 1.0

Item {
    id: root

    // C++
    property string email: "test.email@mega.co.nz"
    property string password: "insecure"
    property bool emailConfirmed: false

    // Mockup
    property bool require2FA: true

    signal loginFinished(int errorCode, string errorMsg)
    signal registerFinished(bool success)
    signal changeRegistrationEmailFinished(bool success)
    signal fetchingNodesProgress(double progress)
    signal fetchingNodesFinished(bool firstTime)
    signal accountCreationResumed
    signal logoutByUser
    signal logoutBySdk
    signal accountCreateCancelled
    signal goToSignupPage
    signal goToLoginPage
    signal logout

    function login(user, pass) {
        console.debug("mockup LoginController::login() : user -> " + user);
        loginTimer.start();
    }

    function createAccount(email, pass, name, lastname) {
        console.debug("mockup LoginController::onRegisterClicked() -> "
                      + email + " : " + name + " : " + lastname);
        registerTimer.start();
    }

    function changeRegistrationEmail(email) {
        console.debug("mockup LoginController::changeRegistrationEmail() : email -> "
                      + email);
        changeEmailTimer.start();
    }

    function login2FA(pin) {
        console.debug("mockup LoginController::login2FA() : pin -> " + pin);
        require2FA = false;
        loginTimer.start();
    }

    function cancelLogin() {
        console.debug("mockup LoginController::cancelLogin()");
    }

    function cancelCreateAccount() {
        console.debug("mockup LoginController::cancelCreateAccount()");
        accountCreateCancelled();
    }

    Timer {
        id: loginTimer

        interval: 2000;
        running: false;
        repeat: false;
        onTriggered: {
            if(require2FA) {
                loginFinished(ApiEnums.API_EMFAREQUIRED, "");
            } else {
                loginFinished(ApiEnums.API_OK, "");
                fetchNodesTimer.start();
            }
        }
    }

    Timer {
        id: fetchNodesTimer

        interval: 2000;
        running: false;
        repeat: false;
        triggeredOnStart: true
        onTriggered: {
            if(running) {
                fetchingNodesProgress(0.5);
            } else {
                fetchingNodesProgress(1);
                fetchingNodesFinished(true);
            }
        }
    }

    Timer {
        id: registerTimer

        interval: 2000;
        running: false;
        repeat: false;
        onTriggered: {
            registerFinished(true);
            accountConfirmTimer.start();
        }
    }

    Timer {
        id: accountConfirmTimer

        interval: 2000;
        running: false;
        repeat: false;
        onTriggered: {
            emailConfirmed = true;
        }
    }

    Timer {
        id: changeEmailTimer

        interval: 2000;
        running: false;
        repeat: false;
        onTriggered: {
            changeRegistrationEmailFinished(true);
        }
    }

}
