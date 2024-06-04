import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.buttons 1.0
import components.steps 1.0

import onboard 1.0

Rectangle {
    id: root

    readonly property string step1DeviceName: "step1DeviceName"
    readonly property string step2InstallationType: "step2InstallationType"
    readonly property string step3: "step3"
    readonly property string step4: "step4"
    readonly property string step4Warning: "step4Warning"
    readonly property string step4Error: "step4Error"
    readonly property string stepAllDone: "stepAllDone"

    readonly property int lineLeftMargin: 15
    readonly property int subStepLeftMargin: 4
    readonly property int lineWidth: 2
    readonly property int lineMainStepHeight: 28
    readonly property int lineSubStepHeight: 16
    readonly property int lineRadius: 1

    property alias step3Text: step3_content.text
    property alias step4Text: step4_content.text

    height: parent.height
    color: ColorTheme.surface1

    state: root.step1DeviceName
    states: [
        State {
            name: root.step1DeviceName
            PropertyChanges { target: step1_deviceName; toState: Step.ToStates.CURRENT; }
            PropertyChanges { target: step2_line; color: ColorTheme.buttonDisabled; }
            PropertyChanges { target: step2_installationType; toState: Step.ToStates.DISABLED; }
            PropertyChanges { target: step3_line; visible: false; }
            PropertyChanges { target: step3_content; visible: false; }
            PropertyChanges { target: step4_line; visible: false; }
            PropertyChanges { target: step4_content; visible: false; }
        },
        State {
            name: root.step2InstallationType
            extend: root.step1DeviceName
            PropertyChanges { target: step1_deviceName; toState: Step.ToStates.DONE; }
            PropertyChanges { target: step2_line; color: ColorTheme.buttonPrimary; }
            PropertyChanges { target: step2_installationType; toState: Step.ToStates.CURRENT; }
        },
        State {

            name: root.step3
            extend: root.step2InstallationType
            PropertyChanges { target: step2_installationType; toState: Step.ToStates.CURRENT_SUBSTEP; }
            PropertyChanges { target: step3_line; color: ColorTheme.buttonPrimary; visible: true; }
            PropertyChanges { target: step3_content; toState: SubStep.ToStates.CURRENT; visible: true; }
            PropertyChanges { target: step4_line; color: ColorTheme.buttonDisabled; visible: true; }
            PropertyChanges { target: step4_content; toState: SubStep.ToStates.DISABLED; visible: true; }
        },
        State {
            name: root.step4
            extend: root.step3
            PropertyChanges { target: step3_content; toState: SubStep.ToStates.DONE; }
            PropertyChanges { target: step4_line; color: ColorTheme.iconButton; }
            PropertyChanges { target: step4_content; toState: SubStep.ToStates.CURRENT; }
        },
        State {
            name: root.step4Warning
            extend: root.step4
            PropertyChanges { target: step4_content; toState: SubStep.ToStates.WARNING; }
        },
        State {
            name: root.step4Error
            extend: root.step4
            PropertyChanges { target: step4_content; toState: SubStep.ToStates.ERROR; }
        },
        State {
            name: root.stepAllDone
            extend: root.step4
            PropertyChanges { target: step2_installationType; toState: Step.ToStates.DONE; }
            PropertyChanges { target: step4_content; toState: SubStep.ToStates.DONE; }
        }
    ]

    ColumnLayout {
        id: stepsLayout

        anchors {
            top: parent.top
            right: parent.right
            left: parent.left
        }
        spacing: 4

        Step {
            id: step1_deviceName

            Layout.alignment: Qt.AlignTop
            number: 1
            text: OnboardingStrings.deviceName
            Layout.fillWidth: true
        }

        Rectangle {
            id: step2_line

            Layout.preferredWidth: root.lineWidth
            Layout.preferredHeight: root.lineMainStepHeight
            Layout.leftMargin: root.lineLeftMargin
            Layout.alignment: Qt.AlignTop
            color: ColorTheme.buttonSecondaryPressed
            radius: root.lineRadius
        }

        Step {
            id: step2_installationType

            Layout.alignment: Qt.AlignTop
            number: 2
            text: OnboardingStrings.setUpOptions
            Layout.fillWidth: true
        }

        Rectangle {
            id: step3_line

            Layout.preferredWidth: root.lineWidth
            Layout.preferredHeight: root.lineSubStepHeight
            Layout.leftMargin: root.lineLeftMargin
            Layout.alignment: Qt.AlignTop
            color: ColorTheme.buttonSecondaryPressed
            radius: root.lineRadius
        }

        SubStep {
            id: step3_content

            Layout.leftMargin: root.subStepLeftMargin
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
        }

        Rectangle {
            id: step4_line

            Layout.preferredWidth: root.lineWidth
            Layout.preferredHeight: root.lineSubStepHeight
            Layout.leftMargin: root.lineLeftMargin
            Layout.alignment: Qt.AlignTop
            color: ColorTheme.buttonSecondaryPressed
            radius: root.lineRadius
        }

        SubStep {
            id: step4_content

            Layout.leftMargin: root.subStepLeftMargin
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
        }

    } // ColumnLayout: stepsLayout

    IconButton {
        id: helpButton

        anchors {
            bottom: parent.bottom
            left: parent.left
            leftMargin: -helpButton.sizes.horizontalAlignWidth
            bottomMargin: -helpButton.sizes.verticalAlignWidth
        }
        icons.source: Images.helpCircle
        sizes.iconSize: Qt.size(24, 24)
        onClicked: {
            Qt.openUrlExternally(Links.desktopSyncApp);
        }
    }
}
