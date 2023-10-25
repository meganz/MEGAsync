// System
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

// QML common
import Common 1.0
import Components.Buttons 1.0 as MegaButtons

// Local
import Onboard 1.0

Rectangle {
    id: root

    readonly property string step1DeviceName: "STEP1_DEVICE_NAME"
    readonly property string step2InstallationType: "STEP2_INSTALLATION_TYPE"
    readonly property string step3: "STEP3"
    readonly property string step4: "STEP4"
    readonly property string step4Warning: "STEP4_WARNING"
    readonly property string step4Error: "STEP4_ERROR"
    readonly property string stepAllDone: "ALL_DONE"

    readonly property int lineLeftMargin: 15
    readonly property int subStepLeftMargin: 4
    readonly property int lineWidth: 2
    readonly property int lineMainStepHeight: 28
    readonly property int lineSubStepHeight: 16
    readonly property int lineRadius: 1

    property alias step3Text: step3_content.text
    property alias step4Text: step4_content.text

    color: Styles.surface1
    height: parent.height

    state: step1DeviceName

    states: [
        State {
            name: step1DeviceName
            PropertyChanges {
                target: step1_deviceName;
                toState: Step.ToStates.Current;
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.iconButtonDisabled;
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.Disabled;
            }
            PropertyChanges {
                target: step3_line;
                visible: false;
            }
            PropertyChanges {
                target: step3_content;
                visible: false;
            }
            PropertyChanges {
                target: step4_line;
                visible: false;
            }
            PropertyChanges {
                target: step4_content;
                visible: false;
            }
        },
        State {
            name: step2InstallationType
            PropertyChanges {
                target: step1_deviceName;
                toState: Step.ToStates.Done;
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.iconButton;
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.Current;
            }
            PropertyChanges {
                target: step3_line;
                visible: false;
            }
            PropertyChanges {
                target: step3_content;
                visible: false;
            }
            PropertyChanges {
                target: step4_line;
                visible: false;
            }
            PropertyChanges {
                target: step4_content;
                visible: false;
            }
        },
        State {
            name: step3
            PropertyChanges {
                target: step1_deviceName;
                toState: Step.ToStates.Done;
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.iconButton;
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.CurrentSubstep;
            }
            PropertyChanges {
                target: step3_line;
                color: Styles.iconButton;
                visible: true;
            }
            PropertyChanges {
                target: step3_content;
                toState: SubStep.ToStates.Current;
                visible: true;
            }
            PropertyChanges {
                target: step4_line;
                color: Styles.iconButtonDisabled;
                visible: true;
            }
            PropertyChanges {
                target: step4_content;
                toState: SubStep.ToStates.Disabled;
                visible: true;
            }
        },
        State {
            name: step4
            PropertyChanges {
                target: step1_deviceName;
                toState: Step.ToStates.Done;
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.iconButton;
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.CurrentSubstep;
            }
            PropertyChanges {
                target: step3_line;
                color: Styles.iconButton;
                visible: true;
            }
            PropertyChanges {
                target: step3_content;
                toState: SubStep.ToStates.Done;
                visible: true;
            }
            PropertyChanges {
                target: step4_line;
                color: Styles.iconButton;
                visible: true;
            }
            PropertyChanges {
                target: step4_content;
                toState: SubStep.ToStates.Current;
                visible: true;
            }
        },
        State {
            name: step4Warning
            PropertyChanges {
                target: step1_deviceName;
                toState: Step.ToStates.Done;
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.iconButton;
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.CurrentSubstep;
            }
            PropertyChanges {
                target: step3_line;
                color: Styles.iconButton;
                visible: true;
            }
            PropertyChanges {
                target: step3_content;
                toState: SubStep.ToStates.Done;
                visible: true;
            }
            PropertyChanges {
                target: step4_line;
                color: Styles.iconButton;
                visible: true;
            }
            PropertyChanges {
                target: step4_content;
                toState: SubStep.ToStates.Warning;
                visible: true;
            }
        },
        State {
            name: step4Error
            PropertyChanges {
                target: step1_deviceName;
                toState: Step.ToStates.Done;
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.iconButton;
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.CurrentSubstep;
            }
            PropertyChanges {
                target: step3_line;
                color: Styles.iconButton;
                visible: true;
            }
            PropertyChanges {
                target: step3_content;
                toState: SubStep.ToStates.Done;
                visible: true;
            }
            PropertyChanges {
                target: step4_line;
                color: Styles.iconButton;
                visible: true;
            }
            PropertyChanges {
                target: step4_content;
                toState: SubStep.ToStates.Error;
                visible: true;
            }
        },
        State {
            name: stepAllDone
            PropertyChanges {
                target: step1_deviceName;
                toState: Step.ToStates.Done;
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.iconButton;
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.Done;
            }
            PropertyChanges {
                target: step3_line;
                color: Styles.iconButton;
                visible: true;
            }
            PropertyChanges {
                target: step3_content;
                toState: SubStep.ToStates.Done;
                visible: true;
            }
            PropertyChanges {
                target: step4_line;
                color: Styles.iconButton;
                visible: true;
            }
            PropertyChanges {
                target: step4_content;
                toState: SubStep.ToStates.Done;
                visible: true;
            }
        }
    ]

    ColumnLayout {
        id: stepsLayout

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 4

        Step {
            id: step1_deviceName

            number: 1
            text: OnboardingStrings.deviceName
            Layout.alignment: Qt.AlignTop
        }

        Rectangle {
            id: step2_line

            color: Styles.buttonSecondaryPressed
            Layout.preferredWidth: lineWidth
            Layout.preferredHeight: lineMainStepHeight
            radius: lineRadius
            Layout.leftMargin: lineLeftMargin
            Layout.alignment: Qt.AlignTop
        }

        Step {
            id: step2_installationType

            number: 2
            text: OnboardingStrings.setUpOptions
            Layout.alignment: Qt.AlignTop
        }

        Rectangle {
            id: step3_line

            color: Styles.buttonSecondaryPressed
            Layout.preferredWidth: lineWidth
            Layout.preferredHeight: lineSubStepHeight
            radius: lineRadius
            Layout.leftMargin: lineLeftMargin
            Layout.alignment: Qt.AlignTop
        }

        SubStep {
            id: step3_content

            Layout.leftMargin: subStepLeftMargin
            Layout.alignment: Qt.AlignTop
        }

        Rectangle {
            id: step4_line

            color: Styles.buttonSecondaryPressed
            Layout.preferredWidth: lineWidth
            Layout.preferredHeight: lineSubStepHeight
            radius: lineRadius
            Layout.leftMargin: lineLeftMargin
            Layout.alignment: Qt.AlignTop
        }

        SubStep {
            id: step4_content

            Layout.leftMargin: subStepLeftMargin
            Layout.alignment: Qt.AlignTop
        }
    }

    MegaButtons.IconButton {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        icons.source: Images.helpCircle
        sizes.iconSize: Qt.size(24, 24)
        onClicked: {
            Qt.openUrlExternally(Links.desktopSyncApp);
        }
    }
}
