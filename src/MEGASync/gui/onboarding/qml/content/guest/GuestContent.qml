// System
import QtQuick 2.12

// QML common
import Common 1.0

// C++
import GuestController 1.0

GuestContentForm {

    GuestController {
        id: controller
    }

    loginButton.onClicked: {
        if(registerFlow.state === register) {
            registerFlow.state = login;
        }
        onboardingWindow.visible = true;
        guestWindow.visible = false;
    }

    signUpButton.onClicked: {
        if(registerFlow.state === login || registerFlow.state === twoFA) {
            registerFlow.state = register;
        }
        onboardingWindow.visible = true;
        guestWindow.visible = false;
    }

    menuButton.onClicked: {
        menuButton.checked = true;
        menu.visible = !menu.visible;
    }

    menu.onClosed: {
        menuButton.checked = false;
    }

    aboutMenuItem.onTriggered: {
        controller.onAboutMEGAClicked();
    }

    preferencesItem.onTriggered: {
        controller.onPreferencesClicked();
    }

    exitItem.onTriggered: {
        controller.onExitClicked();
    }

    Connections {
        target: loginController

        onFetchingNodesProgress: {
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
