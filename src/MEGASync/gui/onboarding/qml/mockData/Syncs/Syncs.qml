import QtQuick 2.0

Item {
    id: root
    signal cantSync
    signal syncSetupSuccess

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
}
