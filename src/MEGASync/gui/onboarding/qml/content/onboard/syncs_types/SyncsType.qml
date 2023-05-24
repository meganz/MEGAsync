// System
import QtQuick 2.12

QtObject {

    enum Types {
        None = 0,
        Sync = 1,
        Backup = 2,
        Fuse = 3
    }

    enum SyncTypes {
        NoSync = 0,
        FullSync = 1,
        SelectiveSync = 2
    }

    property int type: SyncsType.Types.Sync
    property int subType: SyncsType.SyncTypes.FullSync

}
