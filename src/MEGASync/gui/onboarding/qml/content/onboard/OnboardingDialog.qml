import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12

import Components 1.0
import com.qmldialog 1.0 as Cpp
import Onboarding 1.0
import Onboard.Syncs_types 1.0
import Onboard.Syncs_types.Syncs 1.0

Cpp.QmlDialog {
    objectName: "app1"
    title: "Set up MEGA"
    modality: Qt.NonModal
    width: 776
    height: 544

    OnboardingFlow {
        id: onboarding

        anchors.fill: parent
    }

    Connections {
        target: Onboarding

        onNotNowFinished: {
            Wrapper.accept();
        }
    }
}

