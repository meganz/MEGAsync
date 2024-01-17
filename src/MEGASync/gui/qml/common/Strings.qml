pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string cancel: qsTr("Cancel")
    readonly property string done: qsTr("Done")
    readonly property string next: qsTr("Next")
    readonly property string skip: qsTr("Skip")
    readonly property string previous: qsTr("Previous")
    readonly property string tryAgain: qsTr("Try again")
    readonly property string viewInSettings: qsTr("View in Settings")

}
