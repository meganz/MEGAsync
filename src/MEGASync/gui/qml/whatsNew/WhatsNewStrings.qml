pragma Singleton
import QtQuick 2.15

import common 1.0

QtObject {

    readonly property string whatsNew: qsTr("What's new")
    readonly property string updates: qsTr("Updates in the MEGA desktop app")
    readonly property string leftTitle: qsTr("Better Performance")
    readonly property string middleTitle: qsTr("Greater control")
    readonly property string rightTitle: qsTr("Advanced filters")
    readonly property string leftDescription: qsTr("Files now sync up to 5x faster than before")
    readonly property string middleDescription: qsTr("We now provide full visibility into sync issues and give you total control over how conflicts are resolved")
    readonly property string rightDescription: qsTr("Customise your sync with a whole new interface to exclude files and folders. Advanced settings allow you to write your own exclusion rules for each of your syncs")
    readonly property string gotIt: qsTr("Got it")
}
