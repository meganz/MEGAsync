// System
import QtQuick 2.12
import QtQuick.Window 2.12

// QML common
import com.qmldialog 1.0

QmlDialog {

    property bool loggingIn: true

    signal closingButLoggingIn

    function forceClose() {
        console.debug("mockup OnboardingQmlDialog: forceClose()");
    }

}
