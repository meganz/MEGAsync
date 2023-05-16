// System
import QtQuick 2.12

QtObject {

    enum Types {
        Sync = 0,
        Backup = 1,
        Fuse = 2
    }

    enum SyncTypes {
        FullSync = 0,
        SelectiveSync = 1
    }

    property int type: SyncsType.Types.Sync
    property int subType: SyncsType.SyncTypes.FullSync

}
