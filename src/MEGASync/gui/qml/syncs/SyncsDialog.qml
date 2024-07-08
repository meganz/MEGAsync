import QtQuick 2.0

import common 1.0

import components.views 1.0
import components.steps 1.0

import SyncsQmlDialog 1.0

SyncsQmlDialog {
    id: window

    title: SyncsStrings.syncsWindowTitle
    visible: true
    modality: Qt.NonModal
    width: 640
    height: 580
    maximumHeight: 580
    maximumWidth: 640
    minimumHeight: 580
    minimumWidth: 640

    Column {
        id: contentItem

        anchors.fill: parent

        StepPanel {
            id: stepPanelItem

            width: parent.width
            step1String: SyncsStrings.syncType
            step2String: SyncsStrings.sync
            helpUrl: Links.helpSyncs
        }

        Rectangle {
            id: syncsContentItem

            width: parent.width
            height: parent.height - stepPanelItem.height
            color: colorStyle.surface1

            readonly property string syncsFlow: "syncsFlow"
            readonly property string resume: "resume"

            state: syncsFlow
            states: [
                State {
                    name: syncsContentItem.syncsFlow
                    StateChangeScript {
                        script: stackView.replace(syncsFlowPage);
                    }
                },
                State {
                    name: syncsContentItem.resume
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
                    id: syncsFlowPage

                    SyncsPage {
                        id: syncsFlowItem

                        stepPanelRef : stepPanelItem
                        syncsContentItemRef: syncsContentItem
                    }
                }

                Component {
                    id: resumePage

                    ResumeSyncsPage {
                        id: resumeSyncsPageItem

                        footerButtons.leftPrimary.visible: false

                        image.source: Images.syncResume
                        image.sourceSize: Qt.size(128, 128)
                    }
                }
            }

        } // Rectangle: syncsContentItem
    }

}
