import Onboarding 1.0

RegisterPageForm {

    registerButton.onClicked: {

        if (password !== re_password) {
            passwordError = true;
            return;
        } else {
            passwordError = false;
        }

        var formData = {
            [Onboarding.PASSWORD]: password,
            [Onboarding.EMAIL]: email,
            [Onboarding.FIRST_NAME]: name,
            [Onboarding.LAST_NAME]: last_name
        }

        for (let [key, value] of formData) {
            if (value === "") {
                console.log("RegisterPageForm: empty input text");
                return;
            }
        }

        Onboarding.onRegisterClicked(formData);
    }
}
