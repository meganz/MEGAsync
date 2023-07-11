import QtQuick 2.0
import QtQuick.Window 2.12

Window {

    property bool loggingIn: true

    signal closingButLoggingIn

    function onForgotPasswordClicked() {
        console.debug("mockup QmlDialog::onForgotPasswordClicked()");
    }

    function onLoginClicked(data) {
        console.debug("mockup QmlDialog::onLoginClicked() -> "
                      + JSON.stringify(data));
    }

    function accept() {
        console.debug("mockup QmlDialog::accept()");
    }

    function reject() {
        console.debug("mockup QmlDialog::reject()");
    }

    flags: Qt.Dialog
}
