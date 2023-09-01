pragma Singleton
import QtQuick 2.12

import Common 1.0

QtObject {

    // !!!!!!!!!!!!!!!!!!!!!!!!!
    //   Sorted alphabetically
    // !!!!!!!!!!!!!!!!!!!!!!!!!

    readonly property string menuAboutMEGA: qsTr("About MEGA")
    readonly property string menuExit: OS.isMac() ? qsTr("Quit") : qsTr("Exit")
    readonly property string menuSettings: qsTr("Settings")
    readonly property string logInOrSignUp: qsTr("Log in or sign up to MEGA")
    readonly property string accountTempLocked: qsTr("Account temporarily locked")
    readonly property string accountTempLockedSMS: qsTr("Your account has been temporarily locked due to a potential breach of our [a]Terms of Service[/a]. To unlock your account, verify your phone number.")
    readonly property string accountTempLockedEmail: qsTr("Your account has been temporarily locked for your safety. This is due to a potential data breach. To unlock your account, follow the steps in the email we've sent you.")
    readonly property string resendEmail: qsTr("Resend email")
    readonly property string logOut: qsTr("Log out")
    readonly property string verifyNow: qsTr("Verify now")

}
