pragma Singleton
import QtQuick 2.15

import common 1.0

QtObject {

    readonly property string menuAboutMEGA: qsTr("About MEGA")
    readonly property string menuExit: OS.isMac() ? qsTr("Quit") : qsTr("Exit")
    readonly property string menuSettings: qsTr("Settings")
    readonly property string logInOrSignUp: qsTr("Log in or sign up to MEGA")
    readonly property string accountTempLocked: qsTr("Account temporarily locked")
    readonly property string accountTempLockedEmail: qsTr("Your account has been locked for your protection after detecting a malicious login, so we require you to reset your password.[BR]
Check your email inbox for instructions on unlocking your account and tips on how to prevent this from happening again.")
    readonly property string resendEmail: qsTr("Resend email")
    readonly property string logOut: qsTr("Log out")
    readonly property string loggedInOnboarding: qsTr("Setting up your account…")
    readonly property string statusWaitingForEmail: qsTr("Waiting for email confirmation…")
}
