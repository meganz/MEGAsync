pragma Singleton
import QtQuick 2.0

Item {
    enum OnboardEnum {
        FIRST_NAME = 0,
        LAST_NAME = 1,
        EMAIL = 2,
        PASSWORD = 3
    }

    signal userPassFailed
    signal twoFARequired
    signal loginFinished
    signal notNowFinished

    function onForgotPasswordClicked() {
        console.info("onForgotPasswordClicked()");
    }

    function onLoginClicked(data) {
        console.info("onLoginClicked() -> " + JSON.stringify(data));
        twoFARequired();
    }

    function onRegisterClicked(data) {
        console.info("onRegisterClicked() -> " + JSON.stringify(data));
    }

    function onTwoFACompleted(key) {
        console.info("onTwoFACompleted() -> key: " + key);
        loginFinished();
    }

    function onNotNowClicked() {
        console.info("onNotNowClicked()");
        notNowFinished();
    }

    function getComputerName() {
        console.info("getComputerName()");
        return "My PC name";
    }
}
