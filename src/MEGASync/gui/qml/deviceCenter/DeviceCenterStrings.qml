pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string windowTitle: qsTr("Device Centre")
    readonly property string addBackupLabel: qsTr("Add Backup")
    readonly property string addSyncLabel: qsTr("Add Sync")
}
