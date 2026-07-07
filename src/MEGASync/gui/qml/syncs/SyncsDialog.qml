import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

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
    minimumHeight: syncsContentItem.implicitHeight
    maximumHeight: syncsContentItem.implicitHeight
    height: minimumHeight
    width: 640
    maximumWidth: width
    minimumWidth: width
    closeOnEscapePressed: true
    color: ColorTheme.surface1

    readonly property int defaultWindowMargin: 24

    Behavior on height {
        // Only animate height changes once the dialog has finished its
        // initial show-and-position phase. Otherwise the multi-pass QML
        // layout would animate from the first computed implicitHeight to
        // the final one while the user can already see the dialog,
        // producing a visible "shrink" glitch.
        enabled: window.initialLayoutComplete
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    Rectangle {
        id: syncsContentItem

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        color: ColorTheme.surface1

        implicitHeight : stackView.implicitHeight
        height: implicitHeight

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
                left: parent.left
                right: parent.right
                top: parent.top
                margins: defaultWindowMargin
            }

            implicitHeight: currentItem.implicitHeight

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
