import QtQuick 2.0

Item {
    id: root

    signal cantSync(string message, bool localFolderError)
    signal syncSetupSuccess
    signal cancelSync

    function addSync(localPath , remoteHandle) {
        console.info("addSync()" + " localPath:" + localPath + " remoteHandle:" + remoteHandle)
        addSyncTimer.start();
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

}
