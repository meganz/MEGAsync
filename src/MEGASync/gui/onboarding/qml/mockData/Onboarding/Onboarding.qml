pragma Singleton
import QtQuick 2.0

import AccountInfoData 1.0
import Components.TextFields 1.0 as MegaTextFields

Item {

    property string email: "test.email@mega.co.nz"

    signal exitLoggedInFinished
    signal backupsUpdated(string path, int errorCode, bool finished)
    signal backupConflict(string folder, bool isNew)
    signal deviceNameReady
    signal accountBlocked
    signal fetchingNodesProgress(double progress)
    signal showLoginPage
    signal showSignUpPage
    signal hideGuestWindow
    signal showGuestWindow
    signal logout

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
           // accountConfirmTimer.start();
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

    function getPasswordStrength(password) {
        console.info("getPasswordStrength(password)" + password);
        var strength = password.length - 1;
        return strength > MegaTextFields.PasswordTextField.PasswordStrength.PasswordStrengthStrong
               ? MegaTextFields.PasswordTextField.PasswordStrength.PasswordStrengthStrong
               : strength;
    }

    Timer {
        id: addSyncTimer

        interval: 2000;
        running: false;
        repeat: false;
        onTriggered: {
            syncSetupSuccess();
        }
    }

    function addSync(localPath , remoteHandle) {
        console.info("addSync()" + " localPath:" + localPath + " remoteHandle:" + remoteHandle)
        addSyncTimer.start();
    }

    Timer {
        id: backupTimer

        interval: 2000;
        running: false;
        repeat: false;
        onTriggered: {
            backupsUpdated("C:\\Users\\mega\\Documents", 0, true);
        }
    }

    function addBackups(backupDirs) {
        console.info("addBackups() => " + JSON.stringify(backupDirs));
        backupTimer.start();
    }

    function createNextBackup(name) {
        console.info("createNextBackup() => Processing next backup");

        // Comment/Uncomment the following lines to test different scenarios

        // OK and finished
        backupsUpdated("C:\\Users\\mega\\Documents", 0, true);

        // Rename folder
        //backupsUpdated("C:\\Users\\mega\\Documents", -14, true);
    }

    function openPreferences(sync) {
        console.info("openPreferences() -> " + sync);
    }

    function exitLoggedIn() {
        console.info("exitLoggedIn()");
    }

}
