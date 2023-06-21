import QtQml 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

RegisterPageForm {
    id: registerPage

    nextButton.onClicked: {

        if(registerContent.error()) {
            return;
        }

        nextButton.icons.busyIndicatorVisible = true;
        state = signUpStatus;

        loginController.onRegisterClicked(registerContent.email.text, registerContent.password.text,
                                   registerContent.firstName.text, registerContent.lastName.text);
    }

    nextButton.progress.onAnimationFinished: {
        if(completed) {
            registerContent.clean();
            state = normalStatus;
            nextButton.icons.busyIndicatorVisible = false;
            registerFlow.state = confirmEmail;
        }
    }

    loginButton.onClicked: {
        registerFlow.state = login;
    }

    Connections {
        target: loginController

        onRegisterFinished: (success) => {
            if(success) {
                registerFlow.state = confirmEmail;
                registerContent.clean();
            } else {
                nextButton.progressValue = 0;
                registerContent.showEmailAlreadyExistsError();
            }
            state = normalStatus;
            nextButton.icons.busyIndicatorVisible = false;
        }
    }
}
