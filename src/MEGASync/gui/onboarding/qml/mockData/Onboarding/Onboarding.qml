pragma Singleton
import QtQuick 2.0

Item {
    enum OnboardEnum {
        FIRST_NAME = 0,
        LAST_NAME = 1,
        EMAIL = 2,
        PASSWORD = 3
    }

    function onForgotPasswordClicked() {
        console.info("onForgotPasswordClicked()");
    }

    function onLoginClicked(data) {
        console.info("onLoginClicked() -> " + JSON.stringify(data));
    }

    function onRegisterClicked(data) {
        console.info("onRegisterClicked() -> " + JSON.stringify(data));
    }
}
