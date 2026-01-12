import QtQuick 2.0

import common 1.0
import components.views 1.0

import SyncsComponents 1.0
import SyncInfo 1.0
import ServiceUrls 1.0

SyncsQmlDialog {
    id: window

    title: SyncsStrings.syncsWindowTitle
    visible: false
    modality: Qt.NonModal
    width: 640
    height: 460
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width

    Rectangle {
        id: syncsContentItem

        anchors.fill: parent
        color: ColorTheme.surface1

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

            anchors {
                fill: parent
                margins: Constants.defaultWindowMargin
            }

            Component {
                id: syncsFlowPage

                SyncsPage {
                    id: syncsFlowItem

                    syncsContentItemRef: syncsContentItem
                }
            }

            Component {
                id: resumePage

                ResumeSyncsPage {
                    id: resumeSyncsPageItem

                    footerButtons.leftPrimary.visible: false
                }
            }
        }
    }

}
