pragma Singleton
import QtQuick 2.0

import AccountInfoData 1.0

Item {
    enum OnboardEnum {
        FIRST_NAME = 0,
        LAST_NAME = 1,
        EMAIL = 2,
        PASSWORD = 3
    }

    signal userPassFailed
    signal twoFARequired
    signal loginFinished
    signal notNowFinished
    signal twoFAFailed
    signal syncSetupSucces
    signal backupsUpdated

    function onForgotPasswordClicked() {
        console.info("onForgotPasswordClicked()");
        return "https://mega.nz/recovery";
    }

    function onLoginClicked(data) {
        console.info("onLoginClicked() -> " + JSON.stringify(data));

        // Comment/Uncomment the following lines to test different scenarios

        // Login OK
        loginFinished();

        // Login failed
        //userPassFailed();

        // 2FA activated
        //twoFARequired();
    }

    function onRegisterClicked(data) {
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

    function onNotNowClicked() {
        console.info("onNotNowClicked()");
        notNowFinished();
    }

    function getComputerName() {
        console.info("getComputerName()");
        return "My PC name";
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
}
