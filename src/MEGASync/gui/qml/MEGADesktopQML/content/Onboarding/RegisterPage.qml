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
        OnboardCpp.onRegisterClicked({2: formInfoMap['email'],
                                      3: formInfoMap['password'],
                                      0: formInfoMap['firstName'],
                                      1: formInfoMap['lastName']});
    }
}
