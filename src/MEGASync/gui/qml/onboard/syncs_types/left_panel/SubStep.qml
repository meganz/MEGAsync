import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts

Item {
    id: root

    enum ToStates {
        DISABLED = 0,
        CURRENT = 1,
        DONE = 2,
        WARNING = 3,
        ERROR = 4
    }

    property alias text: stepText.text

    property int toState: SubStep.ToStates.DISABLED

    height: substepContent.height
    width: substepContent.width
    Layout.preferredWidth: substepContent.width
    Layout.preferredHeight: substepContent.height

    onToStateChanged: {
        substepContent.state = substepContent.statesMap.get(toState);
    }

    Rectangle {
        id: substepContent

        readonly property string stateDisabled: "DISABLED"
        readonly property string stateCurrent: "CURRENT"
        readonly property string stateDone: "DONE"
        readonly property string stateWarning: "WARNING"
        readonly property string stateError: "ERROR"

        property var statesMap: new Map([
            [SubStep.ToStates.DISABLED, stateDisabled],
            [SubStep.ToStates.CURRENT, stateCurrent],
            [SubStep.ToStates.DONE, stateDone],
            [SubStep.ToStates.WARNING, stateWarning],
            [SubStep.ToStates.ERROR, stateError]
        ])

        state: stateDisabled
        states: [
            State {
                name: substepContent.stateDisabled
                PropertyChanges { target: substepContent; color: "transparent"; }
                PropertyChanges {
                    target: content;
                    spacing: 15;
                    anchors.leftMargin: 8;
                }
                PropertyChanges {
                    target: stepCircleSmall;
                    border.color: Styles.iconButtonDisabled;
                    color: "transparent";
                    visible: true;
                }
                PropertyChanges { target: outlinedCircle; visible: false; }
                PropertyChanges { target: stepText; color: Styles.iconButtonDisabled; }
            },
            State {
                name: substepContent.stateCurrent
                PropertyChanges { target: substepContent; color: Styles.iconButtonPressedBackground; }
                PropertyChanges { target: content; spacing: 12; }
                PropertyChanges { target: stepCircleSmall; visible: false; }
                PropertyChanges { target: stepCircleBig; color: Styles.iconButton; }
                PropertyChanges { target: outlinedCircle; visible: true; }
                PropertyChanges { target: stepText; color: Styles.iconButton; }
            },
            State {
                name: substepContent.stateDone
                PropertyChanges { target: substepContent; color: "transparent"; }
                PropertyChanges {
                    target: content;
                    spacing: 15;
                    anchors.leftMargin: 8;
                }
                PropertyChanges {
                    target: stepCircleSmall;
                    border.color: Styles.supportSuccess;
                    color: Styles.supportSuccess;
                    visible: true;
                }
                PropertyChanges { target: outlinedCircle; visible: false; }
                PropertyChanges { target: stepText; color: Styles.iconButton; }
            },
            State {
                name: substepContent.stateWarning
                PropertyChanges { target: substepContent; color: Styles.notificationWarning; }
                PropertyChanges { target: content; spacing: 12; }
                PropertyChanges { target: stepCircleSmall; visible: false; }
                PropertyChanges { target: stepCircleBig; color: Styles.textWarning; }
                PropertyChanges { target: outlinedCircle; visible: true; }
                PropertyChanges { target: stepText; color: Styles.textWarning; }
            },
            State {
                name: substepContent.stateError
                PropertyChanges { target: substepContent; color: Styles.notificationError; }
                PropertyChanges { target: content; spacing: 12; }
                PropertyChanges { target: stepCircleSmall; visible: false; }
                PropertyChanges { target: stepCircleBig; color: Styles.textError; }
                PropertyChanges { target: outlinedCircle; visible: true; }
                PropertyChanges { target: stepText; color: Styles.textError; }
            }
        ]

        radius: 12
        color: Styles.iconButtonPressedBackground
        height: 24
        width: content.width + 17
        Layout.preferredWidth: width
        Layout.preferredHeight: height

        RowLayout {
            id: content

            anchors {
                top: parent.top
                left: parent.left
                bottom: parent.bottom
                leftMargin: 5
                topMargin: 4
                bottomMargin: 4
                rightMargin: 8
            }
            spacing: 8

            Rectangle {
                id: stepCircleSmall

                width: 8
                height: 8
                radius: 4
                color: "transparent"
                border {
                    width: 2
                    color: Styles.iconButtonDisabled
                }
            }

            Item {
                id: outlinedCircle

                width: 14
                height: 14

                Rectangle {
                    id: stepCircleBig

                    anchors.fill: parent
                    radius: 8
                    color: Styles.iconButton
                    opacity: 0.4
                }

                Rectangle {
                    id: stepCircleBigInside

                    anchors.centerIn: parent
                    width: 8
                    height: 8
                    radius: 4                    
                    color: stepCircleBig.color
                }
            }

            Texts.Text {
                id: stepText

                color: Styles.iconButton
                font {
                    pixelSize: Texts.Text.Size.SMALL
                    weight: Font.DemiBold
                }
            }

        } // RowLayout: content

    } // Rectangle: substepContent

}
