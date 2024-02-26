import QtQuick 2.0

import common 1.0

import components.views 1.0

import BackupsQmlDialog 1.0

BackupsQmlDialog {
    id: window

    title: BackupsStrings.backupsWindowTitle
    visible: true
    modality: Qt.NonModal
    width: 640
    height: 560
    maximumHeight: 560
    maximumWidth: 640
    minimumHeight: 560
    minimumWidth: 640

    Column {
        id: contentItem

        anchors.fill: parent

        StepPanel {
            id: stepPanelItem

            width: parent.width
            height: 128
        }

        Rectangle {
            id: backupsContentItem

            readonly property string backupsFlow: "backupsFlow"
            readonly property string resume: "resume"

            width: parent.width
            height: parent.height - stepPanelItem.height
            color: colorStyle.surface1

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
                    PropertyChanges { target: stepPanelItem; state: stepPanelItem.stepAllDone; }
                }
            ]

            StackViewBase {
                id: stackView

                readonly property int contentMargin: 48

                anchors {
                    fill: parent
                    topMargin: stackView.contentMargin / 2
                    leftMargin: stackView.contentMargin
                    rightMargin: stackView.contentMargin
                    bottomMargin: stackView.contentMargin
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
