import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12

import Components 1.0

QmlDialog {
    id: mWindow
    objectName: "app1"
    title: "Set up MEGA"
    modality: Qt.NonModal
    width: 776
    height: 544

    OnboardingFlow {
        id: onboarding

        anchors.fill: parent
    }
}

