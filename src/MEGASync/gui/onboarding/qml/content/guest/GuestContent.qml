// System
import QtQuick 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

// C++
import GuestController 1.0

GuestContentForm {

    loginButton.onClicked: {
        LoginControllerAccess.guestWindowLoginClicked();
    }

    signUpButton.onClicked: {
        LoginControllerAccess.guestWindowSignupClicked();
    }

    menuButton.onClicked: {
        menuButton.checked = true;
        menu.visible = !menu.visible;
    }

    menu.onClosed: {
        menuButton.checked = false;
    }

    aboutMenuItem.onTriggered: {
        GuestController.onAboutMEGAClicked();
    }

    preferencesItem.onTriggered: {
        GuestController.onPreferencesClicked();
    }

    exitItem.onTriggered: {
        GuestController.onExitClicked();
    }

    Connections {
        target: LoginControllerAccess

        onLoginStarted: {
            guestContent.indeterminateProgress = true;
            guestContent.state = guestContent.stateInProgress;
            guestContent.infoText = OnboardingStrings.statusLogin;
        }

        onFetchingNodesProgress: {
            guestContent.indeterminateProgress = false;
            guestContent.state = guestContent.stateInProgress;
            guestContent.infoText = OnboardingStrings.statusFetchNodes;
            progressBar.value = progress;
        }

        onFetchingNodesFinished: (firstTime) => {
            state = stateNormal;
        }

        onLogout: {
            state = stateNormal;
        }

        onLoginFinished: (errorCode, errorMsg) => {
            state = stateNormal;
        }

        onRegisterFinished: (success) => {
            state = stateNormal;
        }
    }

}
