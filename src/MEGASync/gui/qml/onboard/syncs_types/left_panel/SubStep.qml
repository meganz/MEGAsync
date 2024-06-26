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
    Layout.preferredHeight: substepContent.height

    onToStateChanged: {
        substepContent.state = substepContent.statesMap.get(toState);
    }

    Item {
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
                PropertyChanges { target: background; color: "transparent"; }
                PropertyChanges {
                    target: content;
                    spacing: 15;
                    anchors.leftMargin: 8;
                }
                PropertyChanges {
                    target: stepCircleSmall;
                    border.color: colorStyle.iconButtonDisabled;
                    color: "transparent";
                    visible: true;
                }
                PropertyChanges { target: outlinedCircle; visible: false; }
                PropertyChanges { target: stepText; color: colorStyle.iconButtonDisabled; }
            },
            State {
                name: substepContent.stateCurrent
                PropertyChanges { target: background; color: colorStyle.iconButtonPressedBackground; }
                PropertyChanges { target: content; spacing: 12; }
                PropertyChanges { target: stepCircleSmall; visible: false; }
                PropertyChanges { target: stepCircleBig; color: colorStyle.iconButton; }
                PropertyChanges { target: outlinedCircle; visible: true; }
                PropertyChanges { target: stepText; color: colorStyle.iconButton; }
            },
            State {
                name: substepContent.stateDone
                PropertyChanges { target: background; color: "transparent"; }
                PropertyChanges {
                    target: content;
                    spacing: 15;
                    anchors.leftMargin: 8;
                }
                PropertyChanges {
                    target: stepCircleSmall;
                    border.color: colorStyle.supportSuccess;
                    color: colorStyle.supportSuccess;
                    visible: true;
                }
                PropertyChanges { target: outlinedCircle; visible: false; }
                PropertyChanges { target: stepText; color: colorStyle.iconButton; }
            },
            State {
                name: substepContent.stateWarning
                PropertyChanges { target: background; color: colorStyle.notificationWarning; }
                PropertyChanges { target: content; spacing: 12; }
                PropertyChanges { target: stepCircleSmall; visible: false; }
                PropertyChanges { target: stepCircleBig; color: colorStyle.textWarning; }
                PropertyChanges { target: outlinedCircle; visible: true; }
                PropertyChanges { target: stepText; color: colorStyle.textWarning; }
            },
            State {
                name: substepContent.stateError
                PropertyChanges { target: background; color: colorStyle.notificationError; }
                PropertyChanges { target: content; spacing: 12; }
                PropertyChanges { target: stepCircleSmall; visible: false; }
                PropertyChanges { target: stepCircleBig; color: colorStyle.textError; }
                PropertyChanges { target: outlinedCircle; visible: true; }
                PropertyChanges { target: stepText; color: colorStyle.textError; }
            }
        ]

        height: content.height + 10
        width: parent.width - 20

        Rectangle {
            id: background

            radius: 12
            color: colorStyle.iconButtonPressedBackground
            anchors.left: parent.left
            width: stepText.contentWidth + 40
            height: content.height + 10
        }

        Row {
            id: content

            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
                leftMargin: 5
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
                    color: colorStyle.iconButtonDisabled
                }

                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                id: outlinedCircle

                width: 14
                height: 14
                anchors.verticalCenter: parent.verticalCenter

                Rectangle {
                    id: stepCircleBig

                    anchors.fill: parent
                    radius: 8
                    color: colorStyle.iconButton
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

                width: stepText.implicitWidth > (substepContent.width - 40) ? (substepContent.width - 40) : stepText.implicitWidth
                color: colorStyle.iconButton
                font {
                    pixelSize: Texts.Text.Size.SMALL
                    weight: Font.DemiBold
                }
            }

        } // RowLayout: content

    } // Rectangle: substepContent
}
