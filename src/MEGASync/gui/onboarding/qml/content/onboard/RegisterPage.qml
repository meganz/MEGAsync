import QtQml 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

// C++
import Login 1.0

RegisterPageForm {
    id: registerPage

    nextButton.onClicked: {

        if(registerContent.error()) {
            return;
        }

        nextButton.icons.busyIndicatorVisible = true;
        state = signUpStatus;

        var formData = {
            [Login.PASSWORD]: registerContent.password.text,
            [Login.EMAIL]: registerContent.email.text,
            [Login.FIRST_NAME]: registerContent.firstName.text,
            [Login.LAST_NAME]: registerContent.lastName.text
        }

        loginCpp.onRegisterClicked(formData);
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
        target: loginCpp

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
