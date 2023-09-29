// System
import QtQuick 2.12

QtObject {
    property int typeSelected: SyncsType.Types.None
    property int previousTypeSelected: SyncsType.Types.None
    property bool comesFromResumePage: false
    property bool fullSyncDone: false
    property bool selectiveSyncDone: false
}

