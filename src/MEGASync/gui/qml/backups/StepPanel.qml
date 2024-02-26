import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.steps 1.0

Rectangle {
    id: root

    readonly property string step1: "step1"
    readonly property string step2: "step2"
    readonly property string step2Warning: "step2Warning"
    readonly property string step2Error: "step2Error"
    readonly property string stepAllDone: "stepAllDone"

    readonly property int contentLeftMargin: 48
    readonly property int contentSpacing: 8
    readonly property int lineWidth: 127
    readonly property int lineHeight: 2
    readonly property int lineRadius: 1

    color: colorStyle.pageBackground

    state: root.step1
    states: [
        State {
            name: root.step1
            PropertyChanges { target: step1; toState: Step.ToStates.CURRENT; }
            PropertyChanges { target: line; color: colorStyle.iconButtonDisabled; }
            PropertyChanges { target: step2; toState: Step.ToStates.DISABLED; }
        },
        State {
            name: root.step2
            extend: root.step1
            PropertyChanges { target: step1; toState: Step.ToStates.DONE; }
            PropertyChanges { target: line; color: colorStyle.iconButton; }
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
            name: root.stepAllDone
            extend: root.step2
            PropertyChanges { target: step2; toState: Step.ToStates.DONE; }
        }
    ]

    RowLayout {
        id: stepsLayout

        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: root.contentLeftMargin
        spacing: root.contentSpacing

        Step {
            id: step1

            number: 1
            text: BackupsStrings.selectFolders
        }

        Rectangle {
            id: line

            Layout.preferredWidth: root.lineWidth
            Layout.preferredHeight: root.lineHeight
            color: colorStyle.buttonSecondaryPressed
            radius: root.lineRadius
        }

        Step {
            id: step2

            number: 2
            text: BackupsStrings.confirmFolders
        }

    } // RowLayout: stepsLayout

}
