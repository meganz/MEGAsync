import QtQuick 2.0

import ApiEnums 1.0

Item {
    id: root

    function changeRegistrationEmail(newEmail) {
        console.info("mockup LoginController::changeRegistrationEmail(newEmail): " + newEmail);
        changeEmailTimer.start();
    }

    function onExitLoggedInClicked() {
        console.info("mockup LoginController::onExitLoggedInClicked()");
        exitLoggedInFinished();
    }

    function getComputerName() {
        console.info("mockup LoginController::getComputerName()");
        return "My PC name";
    }

    function onForgotPasswordClicked() {
        console.info("mockup LoginController::onForgotPasswordClicked()");
        return "https://mega.nz/recovery";
    }

    function login(user, pass) {
        console.info("mockup LoginController::login() -> " + user + " " + pass);

        // Comment/Uncomment the following lines to test different scenarios

        // Login OK
        loginTimer.start();

        // Login failed
        //userPassFailed();

        // 2FA activated
        //twoFARequired();
    }

    function createAccount(email, pass, name, lastname) {
        registerTimer.start();
        console.info("mockup LoginController::onRegisterClicked() -> "
                     + email + " " + pass + " " + name + " " + lastname);
    }

    function onTwoFARequested(key) {
        console.info("mockup LoginController::onTwoFARequested() -> key: " + key);

        // Comment/Uncomment the following lines to test different scenarios

        // Login OK
        loginFinished(ApiEnums.API_OK, "");

        // Login failed
        //loginFinished(ApiEnums.API_EEXPIRED, "");
    }

    property string email: "test.email@mega.co.nz"
    property bool emailConfirmed: false

    signal userPassFailed
    signal twoFARequired
    signal loginFinished(int errorCode, string errorMsg)
    signal registerFinished(bool success)
    signal accountConfirmed
    signal logout
    signal fetchingNodesFinished(bool firstTime)
    signal accountCreationResumed
    signal changeRegistrationEmailFinished(bool success)
    signal fetchingNodesProgress(double progress)
    signal logoutByUser
    signal logoutBySdk
    signal accountBlocked

    Timer {
        id: loginTimer

        interval: 2000;
        running: false;
        repeat: false;
        onTriggered: {
            loginFinished(ApiEnums.API_OK, "");
            fetchNodesTimer.start();
        }
    }

    Timer {
        id: fetchNodesTimer
        interval: 2000;
        running: false;
        repeat: false;
        triggeredOnStart: true
        onTriggered: {
            if(running)
            {
                fetchingNodesProgress(0.5);
            }
            else
            {
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
            accountConfirmed();
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
