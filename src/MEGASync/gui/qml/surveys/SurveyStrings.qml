pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string excellent: qsTr("Excellent")
    readonly property string okGotIt: qsTr("OK, got it")
    readonly property string poor: qsTr("Poor")
    readonly property string submit: qsTr("Submit")
    readonly property string tellUsMore: qsTr("Tell us more")
    readonly property string title: qsTr("Thank you for your feedback")
    readonly property string description: qsTr("You’re helping to improve MEGA for you and everyone who uses it")

}
