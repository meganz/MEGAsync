import QtQuick 2.15

QtObject {
    property int typeSelected: SyncsType.Types.NONE
    property int previousTypeSelected: SyncsType.Types.NONE
    property bool comesFromResumePage: false
    property bool fullSyncDone: false
    property bool selectiveSyncDone: false
    property bool syncDone: fullSyncDone || selectiveSyncDone
}

