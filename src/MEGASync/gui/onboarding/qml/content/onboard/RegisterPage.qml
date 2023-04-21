// QML common
import Components 1.0
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

RegisterPageForm {

    password.onTextChanged: {
        var strength = Onboarding.getPasswordStrength(password.text)
        console.log(strength);

        switch(strength)
        {
        case Onboarding.PasswordStrength.PASSWORD_STRENGTH_VERYWEAK:
        {
            password.hint.title = OnboardingStrings.tooWeakPasswordTitle;
            password.hint.text = OnboardingStrings.tooWeakPasswordText;
            break;
        }
        case Onboarding.PasswordStrength.PASSWORD_STRENGTH_WEAK:
        {
            password.hint.title = OnboardingStrings.weakPasswordTitle;
            password.hint.text = OnboardingStrings.weakPasswordText;
            break;
        }
        case Onboarding.PasswordStrength.PASSWORD_STRENGTH_MEDIUM:
        {
            password.hint.title = OnboardingStrings.averagePasswordTitle;
            password.hint.text = OnboardingStrings.averagePasswordText;
            break;
        }
        case Onboarding.PasswordStrength.PASSWORD_STRENGTH_GOOD:
        {
            password.hint.title = OnboardingStrings.strongPasswordTitle;
            password.hint.text = OnboardingStrings.strongPasswordText;
            break;
        }
        case Onboarding.PasswordStrength.PASSWORD_STRENGTH_STRONG:
        {
            password.hint.title = OnboardingStrings.excelentPasswordTitle;
            password.hint.text = OnboardingStrings.excelentPasswordText;
            break;
        }
        }
        password.hint.visible = true;
    }

    nextButton.onClicked: {
        var error = firstName.text.length !== 0 && lastName.text.length !== 0;
        firstLastNameDescription.visible = !error;
        firstName.showType = !error;
        lastName.showType = !error;

        var valid = email.valid();
        if(!valid) {
            error = true;
            email.hint.text = OnboardingStrings.errorValidEmail;
        }
        email.showType = !valid;
        email.hint.visible = !valid;

        valid = password.text.length !== 0;
        if(!valid) {
            error = true;
        }
        password.showType = !valid;

        if(confirmPassword.text.length === 0) {
            error = true;
            confirmPassword.showType = true;
            confirmPassword.hint.visible = true;
            confirmPassword.hint.text = OnboardingStrings.errorConfirmPassword;
        } else if(password.text !== confirmPassword.text) {
            error = true;
            confirmPassword.showType = true;
            confirmPassword.hint.visible = true;
            confirmPassword.hint.text = OnboardingStrings.errorPasswordsMatch;
        } else {
            confirmPassword.showType = false;
            confirmPassword.hint.visible = false;
        }

        if(error) {
            return;
        }

        var formData = {
            [Onboarding.PASSWORD]: password.text,
            [Onboarding.EMAIL]: email.text,
            [Onboarding.FIRST_NAME]: firstName.text,
            [Onboarding.LAST_NAME]: lastName.text
        }

        Onboarding.onRegisterClicked(formData);
    }

    loginButton.onClicked: {
        registerFlow.state = login;
    }
}
