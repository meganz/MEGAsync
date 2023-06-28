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
    }

    signUpButton.onClicked: {
        if(registerFlow.state === login || registerFlow.state === twoFA) {
            registerFlow.state = register;
        }
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

}
