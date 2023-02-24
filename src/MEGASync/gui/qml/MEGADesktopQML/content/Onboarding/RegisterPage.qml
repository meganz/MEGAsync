//import Onboarding 1.0

RegisterPageForm{
    registerButton.onClicked: {
        if(formInfoMap['password'] !== formInfoMap['rePassword'])
        {
            passwordError = true;
            return;
        }
        else
        {
            passwordError = false;
        }

        for (var prop in formInfoMap)
        {
            if(formInfoMap[prop].length === 0)
            {
                return;
            }
        }
        console.log(OnboardCpp.RegisterForm.EMAIL)

        OnboardCpp.onLoginClicked({"[OnboardEnum.EMAIL]": formInfoMap['email'],
                                      "[OnboardEnum.PASSWORD]": formInfoMap['password'],
                                      "[OnboardEnum.FIRST_NAME]": formInfoMap['firstName'],
                                      "[OnboardEnum.LAST_NAME]": formInfoMap['lastName']});
    }
}
