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
        DONE = 3,
        WARNING = 4,
        ERROR = 5
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

        readonly property string stateDisabled: "DISABLED"
        readonly property string stateCurrent: "CURRENT"
        readonly property string stateCurrentSubstep: "SUBSTEP"
        readonly property string stateDone: "DONE"
        readonly property string stateWarning: "WARNING"
        readonly property string stateError: "ERROR"

        property var statesMap: new Map([
            [Step.ToStates.DISABLED, stateDisabled],
            [Step.ToStates.CURRENT, stateCurrent],
            [Step.ToStates.CURRENT_SUBSTEP, stateCurrentSubstep],
            [Step.ToStates.DONE, stateDone],
            [Step.ToStates.WARNING, stateWarning],
            [Step.ToStates.ERROR, stateError]
        ])

        width: content.width + 17
        height: 32
        Layout.preferredWidth: width
        Layout.preferredHeight: height
        radius: 16
        color: colorStyle.iconButtonPressedBackground

        state: stateDisabled
        states: [
            State {
                name: stepContent.stateDisabled
                PropertyChanges { target: stepContent; color: "transparent"; }
                PropertyChanges {
                    target: stepCircle;
                    color: "transparent";
                    border.width: 2;
                    border.color: colorStyle.iconButtonDisabled;
                }
                PropertyChanges { target: stepCircleImage; visible: false; }
                PropertyChanges { target: stepCircleText; color: colorStyle.iconButtonDisabled; }
                PropertyChanges { target: stepText; color: colorStyle.iconButtonDisabled; }
            },
            State {
                name: stepContent.stateCurrent
                PropertyChanges { target: stepContent; color: colorStyle.iconButtonPressedBackground; }
                PropertyChanges { target: stepCircle; color: colorStyle.iconButton; border.width: 0; }
                PropertyChanges { target: stepCircleImage; visible: false; }
                PropertyChanges { target: stepCircleText; color: colorStyle.textInverseAccent; }
                PropertyChanges { target: stepText; color: colorStyle.iconButton; }
            },
            State {
                name: stepContent.stateCurrentSubstep
                PropertyChanges { target: stepContent; color: "transparent"; }
                PropertyChanges { target: stepCircle; color: colorStyle.iconButton; border.width: 0; }
                PropertyChanges { target: stepCircleImage; visible: false; }
                PropertyChanges { target: stepCircleText; color: colorStyle.textInverseAccent; }
                PropertyChanges { target: stepText; color: colorStyle.iconButton; }
            },
            State {
                name: stepContent.stateDone
                PropertyChanges { target: stepContent; color: "transparent"; }
                PropertyChanges { target: stepCircle; color: "transparent"; border.width: 0; }
                PropertyChanges {
                    target: stepCircleImage;
                    visible: true;
                    source: Images.checkCircleFilled;
                    color: colorStyle.supportSuccess;
                }
                PropertyChanges { target: stepCircleText; visible: false; }
                PropertyChanges { target: stepText; color: colorStyle.iconButton; }
            },
            State {
                name: stepContent.stateWarning
                PropertyChanges { target: stepContent; color: colorStyle.notificationWarning; }
                PropertyChanges { target: stepCircle; color: "transparent"; border.width: 0; }
                PropertyChanges {
                    target: stepCircleImage;
                    visible: true;
                    source: Images.alertCircleFilled;
                    color: colorStyle.textWarning;
                }
                PropertyChanges { target: stepCircleText; visible: false; }
                PropertyChanges { target: stepText; color: colorStyle.textWarning; }
            },
            State {
                name: stepContent.stateError
                PropertyChanges { target: stepContent; color: colorStyle.notificationError; }
                PropertyChanges { target: stepCircle; color: "transparent"; border.width: 0; }
                PropertyChanges {
                    target: stepCircleImage;
                    visible: true;
                    source: Images.alertCircleFilled;
                    color: colorStyle.textError;
                }
                PropertyChanges { target: stepCircleText; visible: false; }
                PropertyChanges { target: stepText; color: colorStyle.textError; }
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
                color: colorStyle.iconButton

                Texts.Text {
                    id: stepCircleText

                    anchors.centerIn: parent
                    text: number.toString()
                    color: colorStyle.textInverseAccent
                    font.bold: true
                    lineHeight: 16
                    lineHeightMode: Text.FixedHeight
                }

                SvgImage {
                    id: stepCircleImage

                    anchors.centerIn: parent
                    source: Images.check
                    sourceSize: Qt.size(stepCircle.width, stepCircle.height)
                    color: colorStyle.textInverseAccent
                    visible: false
                }
            }

            Texts.Text {
                id: stepText

                color: colorStyle.iconButton
                font.weight: Font.DemiBold
            }

        } // RowLayout: content

    } // Rectangle: stepContent

}
