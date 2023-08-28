pragma Singleton
import QtQuick 2.12

import Common 1.0

QtObject {

    // !!!!!!!!!!!!!!!!!!!!!!!!!
    //   Sorted alphabetically
    // !!!!!!!!!!!!!!!!!!!!!!!!!

    readonly property var menuAboutMEGA: qsTr("About MEGA")
    readonly property var menuExit: OS.isMac() ? qsTr("Quit") : qsTr("Exit")
    readonly property var menuSettings: qsTr("Settings")
    readonly property var logInOrSignUp: qsTr("Log in or sign up to MEGA")
    readonly property var accountTempLocked: qsTr("Account temporarily locked")
    readonly property var accountTempLockedSMS: qsTr("Your account has been temporarily locked due to a potential breach of our Terms of Service. To unlock your account, verify your phone number.")
    readonly property var accountTempLockedEmail: qsTr("Your account has been temporarily locked for your safety. This is due to a potential data breach. To unlock your account, follow the steps in the email we've sent you.")
    readonly property var resendEmail: qsTr("Resend email")
    readonly property var logOut: qsTr("Log out")
    readonly property var verifyNow: qsTr("Verify now")

}
