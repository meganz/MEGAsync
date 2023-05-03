pragma Singleton
import QtQuick 2.0

import AccountInfoData 1.0
import Components 1.0

Item {

    enum RegisterForm {
        FIRST_NAME = 0,
        LAST_NAME = 1,
        EMAIL = 2,
        PASSWORD = 3
    }

    property string email: "test.email@mega.co.nz"

    signal userPassFailed
    signal twoFARequired
    signal loginFinished
    signal registerFinished(bool apiOk)
    signal notNowFinished
    signal twoFAFailed
    signal syncSetupSucces
    signal backupsUpdated
    signal backupConflict
    signal accountConfirmed
    signal changeRegistrationEmailFinished
    signal deviceNameReady

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

    function onNotNowClicked() {
        console.info("onNotNowClicked()");
        notNowFinished();
    }

    function getComputerName() {
        console.info("getComputerName()");
        return "My PC name";
    }

    function setDeviceName(deviceName) {
        console.info("setDeviceName(deviceName)" + deviceName)
        return false;
    }

    function getPasswordStrength(password) {
        console.info("getPasswordStrength(password)" + password);
        var strength = password.length - 1;
        return strength > PasswordTextField.PasswordStrength.PasswordStrengthStrong ? PasswordTextField.PasswordStrength.PasswordStrengthStrong : strength;
    }

    Timer {
        id: addSyncTimer

        interval: 2000;
        running: false;
        repeat: false;
        onTriggered: {
            syncSetupSucces();
        }
    }

    function addSync(localPath : string, remoteHandle : int) {
        console.info("addSync()" + " localPath:" + localPath + " remoteHandle:" + remoteHandle)
        addSyncTimer.start();
    }

    function addBackups(backupDirs) {
        console.info("addBackups() => " + JSON.stringify(backupDirs));

    }

    function createNextBackup(name) {
        console.info("createNextBackup() => Processing next backup");

        // Comment/Uncomment the following lines to test different scenarios

        // OK and finished
        backupsUpdated("C:\\Users\\mega\\Documents", 0, true);

        // Rename folder
        //backupsUpdated("C:\\Users\\mega\\Documents", -14, true);
    }

    Component.onCompleted: {
        console.info("onboard constructed");
    }
}
