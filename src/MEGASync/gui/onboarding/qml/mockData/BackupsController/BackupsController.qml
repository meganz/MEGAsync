pragma Singleton
import QtQuick 2.0

Item {

    function createBackups() {
        console.log("mockup BackupsController::createBackups()");
        backupsCreationFinished();
    }

    signal backupsCreationFinished()

}
