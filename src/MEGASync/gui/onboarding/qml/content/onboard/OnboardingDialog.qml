// System
import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12

// QML common
import Components 1.0 as Custom

// Local
import Onboard.Syncs_types 1.0
import Onboard.Syncs_types.Syncs 1.0

// C++
import com.qmldialog 1.0 as Cpp
import Onboarding 1.0

Cpp.QmlDialog {
    objectName: "app1"
    title: "Set up MEGA"
    visible: true
    modality: Qt.NonModal
    color: "transparent"

    width: 800
    height: 560
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width

    OnboardingFlow {
        id: onboarding

        anchors.fill: parent
    }

    Connections {
        target: Onboarding

        onExitLoggedInFinished: {
            Wrapper.accept();
        }
    }
}



