// System
import QtQuick 2.12

QtObject {

    enum Types {
        None = 0,
        Sync = 1,
        SelectiveSync = 2,
        FullSync = 3,
        Backup = 4
    }

    property int type: SyncsType.Types.Sync

}
