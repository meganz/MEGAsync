import QtQuick 2.0

import common 1.0

import components.views 1.0
import components.steps 1.0

import BackupsComponent 1.0
import SyncInfo 1.0

import ServiceUrls 1.0

SyncsQmlDialog {
    id: window

    readonly property int syncOrigin: SyncInfo.MAIN_APP_ORIGIN

    title: BackupsStrings.backupsWindowTitle
    visible: true
    modality: Qt.NonModal
    width: 640
    height: 560
    maximumHeight: 560
    maximumWidth: 640
    minimumHeight: 560
    minimumWidth: 640
    backup: true

    Column {
        id: contentItem

        anchors.fill: parent

        StepPanel {
            id: stepPanelItem

            width: parent.width
            step1String: BackupsStrings.selectFolders;
            step2String: BackupsStrings.confirmFolders;
            helpUrl: serviceUrlsAccess.getCreateBackupHelpUrl()
        }

        Rectangle {
            id: backupsContentItem

            readonly property string backupsFlow: "backupsFlow"
            readonly property string resume: "resume"

            width: parent.width
            height: parent.height - stepPanelItem.height
            color: ColorTheme.surface1

            state: backupsFlow
            states: [
                State {
                    name: backupsContentItem.backupsFlow
                    StateChangeScript {
                        script: stackView.replace(backupsFlowPage);
                    }
                },
                State {
                    name: backupsContentItem.resume
                    StateChangeScript {
                        script: stackView.replace(resumePage);
                    }
                    PropertyChanges { target: stepPanelItem; state: stepPanelItem.stepCurrentDone; }
                }
            ]

            StackViewBase {
                id: stackView

                anchors {
                    fill: parent
                    margins: Constants.defaultWindowMargin
                }

                Component {
                    id: backupsFlowPage

                    BackupsPage {
                        id: backupsFlowItem

                        stepPanelRef: stepPanelItem
                        backupsContentItemRef: backupsContentItem
                    }
                }

                Component {
                    id: resumePage

                    ResumePage {
                        id: resumePageItem
                    }
                }
            }

        } // Rectangle: backupsContentItem
    }

}
