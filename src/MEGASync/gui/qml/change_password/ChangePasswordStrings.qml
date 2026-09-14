pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string title: qsTr("Change password")
    readonly property string newPassword: qsTr("New password")
    readonly property string confirmNewPassword: qsTr("Confirm new password")
    readonly property string enterNewPassword: qsTr("Enter a new password")
    readonly property string reEnterPassword: qsTr("Re-enter your password")
    readonly property string weakPassword: qsTr("Weak password")
    readonly property string moderatePassword: qsTr("Moderate password")
    readonly property string strongPassword: qsTr("Strong password")
    readonly property string useAtLeast8Characters: qsTr("Use at least 8 characters")
    readonly property string otherSessionsInfo: qsTr("You'll be signed out of all other devices. This MEGA Desktop App session will stay signed in and your syncs won't be affected.")
    readonly property string twoFATitle: qsTr("Enter your 2FA code")
    readonly property string lostAuthenticatorDevice: qsTr("Lost your authenticator device?")
    readonly property string incorrect2FACode: qsTr("Incorrect code, check your authenticator app and try again.")
}
