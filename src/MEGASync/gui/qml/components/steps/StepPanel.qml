import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.buttons 1.0

Rectangle {
    id: root

    readonly property string step1: "step1"
    readonly property string step2: "step2"
    readonly property string step2Warning: "step2Warning"
    readonly property string step2Error: "step2Error"
    readonly property string stepCurrentDone: "stepCurrentDone"
    readonly property string stepAllDone: "stepAllDone"

    readonly property int contentSpacing: 8
    readonly property int lineWidth: 127
    readonly property int lineHeight: 2
    readonly property int lineRadius: 1

    property alias step1String: step1.text
    property alias step2String: step2.text

    property url helpUrl

    height: 104
    color: ColorTheme.pageBackground

    state: root.step1
    states: [
        State {
            name: root.step1
            PropertyChanges { target: step1; toState: Step.ToStates.CURRENT; }
            PropertyChanges { target: line; color: ColorTheme.buttonDisabled; }
            PropertyChanges { target: step2; toState: Step.ToStates.DISABLED; }
        },
        State {
            name: root.step2
            extend: root.step1
            PropertyChanges { target: step1; toState: Step.ToStates.DONE; }
            PropertyChanges { target: line; color: ColorTheme.buttonPrimary; }
            PropertyChanges { target: step2; toState: Step.ToStates.CURRENT; }
        },
        State {
            name: root.step2Warning
            extend: root.step2
            PropertyChanges { target: step2; toState: Step.ToStates.WARNING; }
        },
        State {
            name: root.step2Error
            extend: root.step2
            PropertyChanges { target: step2; toState: Step.ToStates.ERROR; }
        },
        State {
            name: root.stepCurrentDone
            extend: root.step2
            PropertyChanges { target: step2; toState: Step.ToStates.CURRENT_DONE; }
        },
        State {
            name: root.stepAllDone
            extend: root.step2
            PropertyChanges { target: step2; toState: Step.ToStates.DONE; }
        }
    ]

    Item {
        id: contentItem

        anchors {
            fill: parent
            margins: Constants.defaultWindowMargin
        }

        RowLayout {
            id: stepsLayout

            anchors.left: parent.left
            spacing: root.contentSpacing

            Step {
                id: step1

                number: 1
            }

            Rectangle {
                id: line

                Layout.preferredWidth: Math.min((contentItem.width - step2.width - step1.width - (3 * root.contentSpacing) - helpButton.width), root.lineWidth)
                Layout.preferredHeight: root.lineHeight
                color: ColorTheme.buttonSecondaryPressed
                radius: root.lineRadius
            }

            Step {
                id: step2

                number: 2
            }

        } // RowLayout: stepsLayout

        IconButton {
            id: helpButton

            anchors {
                top: parent.top
                right: parent.right
                rightMargin: Constants.focusAdjustment
                topMargin: Constants.focusAdjustment
            }
            icons.source: Images.helpCircle
            onClicked: {
                if (root.helpUrl.toString().length > 0) {
                    Qt.openUrlExternally(root.helpUrl);
                }
            }
        }

    } // Item: contentItem

}
