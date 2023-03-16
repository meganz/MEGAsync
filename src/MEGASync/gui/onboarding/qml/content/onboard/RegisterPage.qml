import Onboarding 1.0

RegisterPageForm {

    cancelButton.onClicked: {
        registerStack.replace(loginPage);
    }

    registerButton.onClicked: {


        if (formData[Onboarding.OnboardEnum.PASSWORD]
                !== re_password) {
            passwordError = true;
            //return;
        } else {
            passwordError = false;
        }

//        for (let [key, value] of formData) {
//            if (value === "") {
//                console.log("RegisterPageForm: empty input text");
//                return;
//            }
//        }

        var formData = {[Onboarding.PASSWORD]: password, [Onboarding.EMAIL]: email, [Onboarding.FIRST_NAME]: name, [Onboarding.LAST_NAME]: last_name}
        Onboarding.onRegisterClicked(formData);
    }
}
