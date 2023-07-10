import QtQuick 2.0
import QtQuick.Window 2.12

Window {

    signal closingButLoggingIn
    flags: Qt.Dialog

    function onForgotPasswordClicked() {
        console.info("onForgotPasswordClicked()");
    }

    function onLoginClicked(data) {
        console.info("onLoginClicked() -> " + JSON.stringify(data));
    }

    function accept() {
        console.log("QmlDialog accepted");
    }

    function reject() {
        console.log("QmlDialog rejected");
    }
}
