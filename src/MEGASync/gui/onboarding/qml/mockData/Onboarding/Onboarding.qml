pragma Singleton
import QtQuick 2.0

import AccountInfoData 1.0
import Components.TextFields 1.0 as MegaTextFields

Item {

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

    function onExitLoggedInClicked() {
        console.info("onExitLoggedInClicked()");
        exitLoggedInFinished();
    }

    property string email: "test.email@mega.co.nz"

    signal exitLoggedInFinished
    signal backupsUpdated(string path, int errorCode, bool finished)
    signal backupConflict(string folder, bool isNew)
    signal deviceNameReady
    signal accountBlocked

    Timer {
        id: backupTimer

        interval: 2000;
        running: false;
        repeat: false;
        onTriggered: {
            backupsUpdated("C:\\Users\\mega\\Documents", 0, true);
        }
    }

}
