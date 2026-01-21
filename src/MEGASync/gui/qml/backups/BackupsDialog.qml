import QtQuick 2.0

import common 1.0

import components.views 1.0
import components.steps 1.0

import BackupsComponent 1.0
import SyncInfo 1.0

import ServiceUrls 1.0

SyncsQmlDialog {
    id: window

    title: BackupsStrings.backupsWindowTitle
    visible: false
    modality: Qt.NonModal
    width: 640
    height: 403
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width
    backup: true
    closeOnEscapePressed: true

    readonly property int defaultWindowMargin: 24

    Column {
        id: contentItem

        anchors.fill: parent

        Rectangle {
            id: backupsContentItem

            readonly property string backupsFlow: "backupsFlow"
            readonly property string resume: "resume"

            width: parent.width
            height: parent.height
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
                }
            ]

            StackViewBase {
                id: stackView

                anchors {
                    fill: parent
                    margins: defaultWindowMargin
                }

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
        }
    }

}
