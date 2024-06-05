import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

Item {
    id: root

    enum ToStates {
        DISABLED = 0,
        CURRENT = 1,
        CURRENT_SUBSTEP = 2,
        CURRENT_DONE = 3,
        DONE = 4,
        WARNING = 5,
        ERROR = 6
    }

    property alias text: stepText.text

    property int number: 0
    property int toState: Step.ToStates.DISABLED

    height: stepContent.height
    width: stepContent.width
    Layout.preferredWidth: stepContent.width
    Layout.preferredHeight: stepContent.height

    onToStateChanged: {
        stepContent.state = stepContent.statesMap.get(toState);
    }

    Rectangle {
        id: stepContent

        readonly property string stateDisabled: "disabled"
        readonly property string stateCurrent: "current"
        readonly property string stateCurrentSubstep: "currentSubstep"
        readonly property string stateCurrentDone: "currentDone"
        readonly property string stateDone: "done"
        readonly property string stateWarning: "warning"
        readonly property string stateError: "error"

        property var statesMap: new Map([
            [Step.ToStates.DISABLED, stateDisabled],
            [Step.ToStates.CURRENT, stateCurrent],
            [Step.ToStates.CURRENT_SUBSTEP, stateCurrentSubstep],
            [Step.ToStates.CURRENT_DONE, stateCurrentDone],
            [Step.ToStates.DONE, stateDone],
            [Step.ToStates.WARNING, stateWarning],
            [Step.ToStates.ERROR, stateError]
        ])

        width: content.width + 17
        height: 32
        Layout.preferredWidth: width
        Layout.preferredHeight: height
        radius: 16
        color: ColorTheme.surface2
        state: stateDisabled
        states: [
            State {
                name: stepContent.stateDisabled
                PropertyChanges { target: stepContent; color: "transparent"; }
                PropertyChanges {
                    target: stepCircle;
                    color: "transparent";
                    border.width: 2;
                    border.color: ColorTheme.buttonDisabled;
                }
                PropertyChanges { target: stepCircleImage; visible: false; }
                PropertyChanges { target: stepCircleText; color: ColorTheme.buttonDisabled; }
                PropertyChanges { target: stepText; color: ColorTheme.buttonDisabled; }
            },
            State {
                name: stepContent.stateCurrent
                PropertyChanges { target: stepContent; color: ColorTheme.iconButtonPressedBackground; }
                PropertyChanges { target: stepCircle; color: ColorTheme.iconButton; border.width: 0; }
                PropertyChanges { target: stepCircleImage; visible: false; }
                PropertyChanges { target: stepCircleText; color: ColorTheme.textInverseAccent; }
                PropertyChanges { target: stepText; color: ColorTheme.buttonPrimary; }
            },
            State {
                name: stepContent.stateCurrentSubstep
                PropertyChanges { target: stepContent; color: "transparent"; }
                PropertyChanges { target: stepCircle; color: ColorTheme.buttonPrimary; border.width: 0; }
                PropertyChanges { target: stepCircleImage; visible: false; }
                PropertyChanges { target: stepCircleText; color: ColorTheme.textInverseAccent; }
                PropertyChanges { target: stepText; color: ColorTheme.buttonPrimary; }
            },
            State {
                name: stepContent.stateDone
                PropertyChanges { target: stepContent; color: "transparent"; }
                PropertyChanges { target: stepCircle; color: "transparent"; border.width: 0; }
                PropertyChanges {
                    target: stepCircleImage;
                    visible: true;
                    source: Images.checkCircleFilled;
                    color: ColorTheme.supportSuccess;
                }
                PropertyChanges { target: stepCircleText; visible: false; }
                PropertyChanges { target: stepText; color: ColorTheme.iconButton; }
            },
            State {
                name: stepContent.stateCurrentDone
                extend: stepContent.stateDone
                PropertyChanges { target: stepContent; color: ColorTheme.iconButtonPressedBackground; }
            },
            State {
                name: stepContent.stateWarning
                extend: stepContent.stateDone
                PropertyChanges { target: stepContent; color: ColorTheme.notificationWarning; }
                PropertyChanges {
                    target: stepCircleImage;
                    source: Images.alertCircleFilled;
                    color: ColorTheme.textWarning;
                }
                PropertyChanges { target: stepText; color: ColorTheme.textWarning; }
            },
            State {
                name: stepContent.stateError
                extend: stepContent.stateWarning
                PropertyChanges { target: stepContent; color: ColorTheme.notificationError; }
                PropertyChanges { target: stepCircleImage; color: ColorTheme.textError; }
                PropertyChanges { target: stepText; color: ColorTheme.textError; }
            }
        ]

        RowLayout {
            id: content

            anchors {
                top: parent.top
                left: parent.left
                bottom: parent.bottom
                leftMargin: 5
                topMargin: 5
                bottomMargin: 5
                rightMargin: 12
            }
            spacing: 8

            Rectangle {
                id: stepCircle

                radius: width / 2
                width: 22
                height: width
                color: ColorTheme.buttonPrimary

                Texts.Text {
                    id: stepCircleText

                    anchors.centerIn: parent
                    text: number.toString()
                    color: ColorTheme.textInverseAccent
                    font.bold: true
                    lineHeight: 16
                    lineHeightMode: Text.FixedHeight
                }

                SvgImage {
                    id: stepCircleImage

                    anchors.centerIn: parent
                    source: Images.check
                    sourceSize: Qt.size(stepCircle.width, stepCircle.height)
                    color: ColorTheme.textInverseAccent
                    visible: false
                }
            }

            Texts.Text {
                id: stepText

                color: ColorTheme.buttonPrimary
                font.weight: Font.DemiBold
            }

        } // RowLayout: content

    } // Rectangle: stepContent

}
