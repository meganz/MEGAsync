import QtQuick 2.0
import QtQuick.Window 2.12

Window {

    property bool loggingIn: true

    function onForgotPasswordClicked() {
        console.info("mockup QmlDialog::onForgotPasswordClicked()");
    }

    function onLoginClicked(data) {
        console.info("mockup QmlDialog::onLoginClicked() -> " + JSON.stringify(data));
    }

    function accept() {
        console.log("mockup QmlDialog::accept()");
    }

    function reject() {
        console.log("mockup QmlDialog::reject()");
    }

    signal closingButLoggingIn

    flags: Qt.Dialog
}
