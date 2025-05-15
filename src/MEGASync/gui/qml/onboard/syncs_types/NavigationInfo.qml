import QtQuick 2.15

import common 1.0

QtObject {

    property int typeSelected: Constants.SyncType.NONE
    property int previousTypeSelected: Constants.SyncType.NONE
    property bool comesFromResumePage: false
    property bool fullSyncDone: false
    property bool selectiveSyncDone: false
    property bool syncDone: fullSyncDone || selectiveSyncDone
    property bool errorOnSyncs: false;

}

