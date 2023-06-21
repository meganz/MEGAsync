import QtQuick 2.0

Item {
    id: root

    property string email: "test.email@mega.co.nz"

    signal userPassFailed
    signal twoFARequired
    signal loginFinished
    signal registerFinished(bool success)
    signal twoFAFailed
    signal accountConfirmed
    signal changeRegistrationEmailFinished(bool success)
    signal fetchingNodesProgress(double progress)

    function onForgotPasswordClicked() {
        console.info("onForgotPasswordClicked()");
        return "https://mega.nz/recovery";
    }

    function getEmail() {
        return email;
    }

    Timer {
        id: loginTimer

        interval: 2000;
        running: false;
        repeat: false;
        onTriggered: {
            loginFinished();
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
            }
        }
    }

    function onLoginClicked(data) {
        console.info("onLoginClicked() -> " + JSON.stringify(data));

        // Comment/Uncomment the following lines to test different scenarios

        // Login OK
        loginTimer.start();

        // Login failed
        //userPassFailed();

        // 2FA activated
        //twoFARequired();
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

    function onRegisterClicked(data) {
        registerTimer.start();
        console.info("onRegisterClicked() -> " + JSON.stringify(data));
    }

    function onTwoFARequested(key) {
        console.info("onTwoFARequested() -> key: " + key);

        // Comment/Uncomment the following lines to test different scenarios

        // Login OK
        loginFinished();

        // Login failed
        //twoFAFailed();
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

    function changeRegistrationEmail(newEmail) {
        console.info("changeRegistrationEmail(newEmail):" + newEmail);
        changeEmailTimer.start();
    }

    function onExitLoggedInClicked() {
        console.info("onExitLoggedInClicked()");
        exitLoggedInFinished();
    }

    function getComputerName() {
        console.info("getComputerName()");
        return "My PC name";
    }
}
