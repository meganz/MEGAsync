pragma Singleton
import QtQuick 2.0

Item {

    signal backupsCreationFinished()

    function createBackups() {
        console.debug("mockup BackupsController::createBackups()");
        backupsCreationFinished();
    }

}
