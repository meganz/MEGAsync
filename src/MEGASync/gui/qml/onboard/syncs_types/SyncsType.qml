import QtQuick 2.15

QtObject {
    id: root

    enum Types {
        NONE = 0,
        SYNC,
        SELECTIVE_SYNC,
        FULL_SYNC,
        BACKUP
    }

    property int type: SyncsType.Types.Sync

}
