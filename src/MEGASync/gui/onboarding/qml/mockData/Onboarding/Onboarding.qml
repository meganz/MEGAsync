pragma Singleton
import QtQuick 2.0

Item {

    signal accountBlocked
    signal logout

    function openPreferences(sync) {
        console.info("mockup Onboarding::openPreferences() -> " + sync);
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

    function exitLoggedIn() {
        console.info("exitLoggedIn()");
    }

}
