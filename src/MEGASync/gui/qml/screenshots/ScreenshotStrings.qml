pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string title: qsTr("Upload screenshots")
    readonly property string offerTitle: qsTr("Automatically save screenshots to MEGA?")
    readonly property string offerDescription: qsTr("Back up your screenshots to the cloud automatically and access them across your devices.")
    readonly property string notNow: qsTr("Not now")
    readonly property string enableScreenshots: qsTr("Enable screenshots")
}
