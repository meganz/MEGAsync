import Onboarding 1.0

RegisterPageForm {

    cancelButton.onClicked: {
        registerStack.replace(loginPage);
    }

    registerButton.onClicked: {
        if (formData.get(Onboarding.OnboardEnum.PASSWORD)
                !== formData.get(re_password)) {
            passwordError = true;
            return;
        } else {
            passwordError = false;
        }

        for (let [key, value] of formData) {
            if (value === "") {
                console.log("RegisterPageForm: empty input text");
                return;
            }
        }

        Onboarding.onRegisterClicked({[Onboarding.OnboardEnum.EMAIL]:
                                        formData.get(Onboarding.OnboardEnum.EMAIL),
                                      [Onboarding.OnboardEnum.PASSWORD]:
                                        formData.get(Onboarding.OnboardEnum.PASSWORD),
                                      [Onboarding.OnboardEnum.FIRST_NAME]:
                                        formData.get(Onboarding.OnboardEnum.FIRST_NAME),
                                      [Onboarding.OnboardEnum.LAST_NAME]:
                                        formData.get(Onboarding.OnboardEnum.LAST_NAME)});
    }
}
