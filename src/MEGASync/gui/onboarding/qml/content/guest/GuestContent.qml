// System
import QtQuick 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

// C++
import GuestController 1.0
import ApiEnums 1.0

GuestContentForm {

    loginButton.onClicked: {
        LoginControllerAccess.guestWindowLoginClicked();
        guestWindow.hide();
    }

    signUpButton.onClicked: {
        LoginControllerAccess.guestWindowSignupClicked();
        guestWindow.hide();
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
        guestWindow.hide();
    }

    preferencesItem.onTriggered: {
        GuestController.onPreferencesClicked();
        guestWindow.hide();
    }

    exitItem.onTriggered: {
        GuestController.onExitClicked();
        guestWindow.hide();
    }

    Connections {
        target: LoginControllerAccess

        onLoginStarted: {
            guestContent.indeterminateProgress = true;
            guestContent.title = OnboardingStrings.statusLogin;
            guestContent.state = guestContent.stateInProgress;
        }

        onLoginFinished: (errorCode, errorMsg) => {
            if(errorCode === ApiEnums.API_OK) {
                return;
            }

            if(errorCode === ApiEnums.API_EMFAREQUIRED) {
                guestContent.indeterminateProgress = true;
                guestContent.state = guestContent.stateInProgress;
                guestContent.title = OnboardingStrings.status2FA;
            } else {
                guestContent.state = guestContent.stateNormal;
            }
        }

        onFetchingNodesProgress: (progress) => {
            if(progess === 0.15) {
                guestContent.indeterminateProgress = false;
                guestContent.title = OnboardingStrings.statusFetchNodes;
                guestContent.state = guestContent.stateInProgress;
            }
            progressBar.value = progress;
        }

        onFetchingNodesFinished: (firstTime) => {
            guestContent.state = guestContent.stateNormal;
        }

        onLogout: {
            guestContent.state = guestContent.stateNormal;
        }

        onRegisterFinished: (success) => {
            guestContent.state = guestContent.stateNormal;
        }
    }

}
