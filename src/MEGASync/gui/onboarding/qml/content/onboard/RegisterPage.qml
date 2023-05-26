import QtQml 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

RegisterPageForm {
    id: registerPage

    nextButton.onClicked: {

        if(registerContent.error()) {
            return;
        }

        nextButton.icons.busyIndicatorVisible = true;
        state = signUpStatus;

        var formData = {
            [Onboarding.RegisterForm.PASSWORD]: registerContent.password.text,
            [Onboarding.RegisterForm.EMAIL]: registerContent.email.text,
            [Onboarding.RegisterForm.FIRST_NAME]: registerContent.firstName.text,
            [Onboarding.RegisterForm.LAST_NAME]: registerContent.lastName.text
        }

        Onboarding.onRegisterClicked(formData);
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
        target: Onboarding

        onRegisterFinished: (success) => {
            if(success) {
                registerFlow.state = confirmEmail;
                registerContent.clean();
            } else {
                nextButton.progressValue = 0;
                registerContent.showEmailAlreadyExistsError();
            }
            state = normalStatus;
            nextButton.busyIndicatorVisible = false;
        }
    }
}
