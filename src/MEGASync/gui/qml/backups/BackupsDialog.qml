import QtQuick 2.0

import common 1.0

import QmlDialog 1.0

QmlDialog {
    id: window

    title: BackupsStrings.setupBackups
    visible: true
    modality: Qt.NonModal
    width: 600
    height: 560
    maximumHeight: 560
    maximumWidth: 600
    minimumHeight: 560
    minimumWidth: 600

    Rectangle {
        id: backgroundRect

        anchors.fill: parent
        color: Styles.surface1

        BackupsFlow {
            id: backupsFlowItem

            anchors.fill: parent
            anchors.margins: 48
        }
    }

    Connections {
        target: backupsFlowItem

        ignoreUnknownSignals: true

        function onBackupFlowMoveToBack() {
            window.close();
        }

        function onBackupFlowMoveToFinal(success) {
            if (success) {
                window.accept();
            }
        }
    }
}
