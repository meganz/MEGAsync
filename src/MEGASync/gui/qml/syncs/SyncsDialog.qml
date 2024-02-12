import QtQuick 2.0

import common 1.0

import components.views 1.0

import QmlDialog 1.0

QmlDialog {
    id: window

    title: SyncsStrings.syncsWindowTitle
    visible: true
    modality: Qt.NonModal
    width: 496
    height: 560
    maximumHeight: 560
    maximumWidth: 496
    minimumHeight: 560
    minimumWidth: 496

    Rectangle {
        id: syncsContentItem

        anchors.fill: parent
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
            }
        ]

        StackViewBase {
            id: stackView

            anchors.fill: parent
            anchors.margins: 48

            Component {
                id: syncsFlowPage

                SyncsFlow {
                    id: syncsFlowItem

                    onSyncsFlowMoveToBack: {
                        window.close();
                    }

                    onSyncsFlowMoveToFinal: {
                        syncsContentItem.state = syncsContentItem.resume;
                    }
                }
            }

            Component {
                id: resumePage

                ResumeSyncsPage {
                    id: resumeSyncsPageItem
                }
            }
        }

    } // Rectangle: syncsContentItem

}
