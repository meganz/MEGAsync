pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string title: qsTr("Change password")
    readonly property string newPassword: qsTr("New password")
    readonly property string confirmNewPassword: qsTr("Confirm new password")
}
