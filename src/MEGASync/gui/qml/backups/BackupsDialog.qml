import QtQuick 2.0

import common 1.0

import components.views 1.0

import BackupsQmlDialog 1.0

BackupsQmlDialog {
    id: window

    title: BackupsStrings.backupsWindowTitle
    visible: true
    modality: Qt.NonModal
    width: 600
    height: 560
    maximumHeight: 560
    maximumWidth: 600
    minimumHeight: 560
    minimumWidth: 600

    Rectangle {
        id: backupsContentItem

        anchors.fill: parent
        color: colorStyle.surface1

        readonly property string backupsFlow: "backupsFlow"
        readonly property string resume: "resume"

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
            }
        ]

        StackViewBase {
            id: stackView

            anchors.fill: parent
            anchors.margins: 48

            Component {
                id: backupsFlowPage

                BackupsPage {
                    id: backupsFlowItem

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

    } // Rectangle: contentItem

}
