import QtQuick 2.0
import QtQuick.Window 2.13

Window {
    function onForgotPasswordClicked() {
        console.info("onForgotPasswordClicked()");
    }

    function onLoginClicked(data) {
        console.info("onLoginClicked() -> " + JSON.stringify(data));
    }
}
