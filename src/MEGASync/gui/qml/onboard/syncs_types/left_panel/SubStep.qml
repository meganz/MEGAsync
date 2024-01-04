import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts

Item {
    id: root

    enum ToStates {
        Disabled = 0,
        Current = 1,
        Done = 2,
        Warning = 3,
        Error = 4
    }

    property alias text: stepText.text
    property int toState: SubStep.ToStates.Disabled

    onToStateChanged: {
        substepContent.state = substepContent.statesMap.get(toState);
    }

    height: substepContent.height
    Layout.preferredHeight: substepContent.height

    Item {
        id: substepContent

        readonly property string stateDisabled: "DISABLED"
        readonly property string stateCurrent: "CURRENT"
        readonly property string stateDone: "DONE"
        readonly property string stateWarning: "WARNING"
        readonly property string stateError: "ERROR"

        property var statesMap: new Map([
            [SubStep.ToStates.Disabled, stateDisabled],
            [SubStep.ToStates.Current, stateCurrent],
            [SubStep.ToStates.Done, stateDone],
            [SubStep.ToStates.Warning, stateWarning],
            [SubStep.ToStates.Error, stateError]
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
                    border.color: Styles.iconButtonDisabled;
                    color: "transparent";
                    visible: true;
                }
                PropertyChanges { target: outlinedCircle; visible: false; }
                PropertyChanges { target: stepText; color: Styles.iconButtonDisabled; }
            },
            State {
                name: substepContent.stateCurrent
                PropertyChanges { target: background; color: Styles.iconButtonPressedBackground; }
                PropertyChanges { target: content; spacing: 12; }
                PropertyChanges { target: stepCircleSmall; visible: false; }
                PropertyChanges { target: stepCircleBig; color: Styles.iconButton; }
                PropertyChanges { target: outlinedCircle; visible: true; }
                PropertyChanges { target: stepText; color: Styles.iconButton; }
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
                    border.color: Styles.supportSuccess;
                    color: Styles.supportSuccess;
                    visible: true;
                }
                PropertyChanges { target: outlinedCircle; visible: false; }
                PropertyChanges { target: stepText; color: Styles.iconButton; }
            },
            State {
                name: substepContent.stateWarning
                PropertyChanges { target: background; color: Styles.notificationWarning; }
                PropertyChanges { target: content; spacing: 12; }
                PropertyChanges { target: stepCircleSmall; visible: false; }
                PropertyChanges { target: stepCircleBig; color: Styles.textWarning; }
                PropertyChanges { target: outlinedCircle; visible: true; }
                PropertyChanges { target: stepText; color: Styles.textWarning; }
            },
            State {
                name: substepContent.stateError
                PropertyChanges { target: background; color: Styles.notificationError; }
                PropertyChanges { target: content; spacing: 12; }
                PropertyChanges { target: stepCircleSmall; visible: false; }
                PropertyChanges { target: stepCircleBig; color: Styles.textError; }
                PropertyChanges { target: outlinedCircle; visible: true; }
                PropertyChanges { target: stepText; color: Styles.textError; }
            }
        ]

        height: content.height + 10
        width: parent.width - 20

        Rectangle {
            id: background

            radius: 12
            color: Styles.iconButtonPressedBackground
            anchors.left: parent.left
            width: stepText.contentWidth + 40
            height: content.height + 10
        }

        Row {
            id: content

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 5
            anchors.rightMargin: 8
            spacing: 8

            Rectangle {
                id: stepCircleSmall

                radius: 4
                width: 8
                height: 8
                border.width: 2
                border.color: Styles.iconButtonDisabled
                color: "transparent"
                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                id: outlinedCircle

                width: 14
                height: 14
                anchors.verticalCenter: parent.verticalCenter

                Rectangle {
                    id: stepCircleBig

                    radius: 8
                    anchors.fill: parent
                    color: Styles.iconButton
                    opacity: 0.4
                }

                Rectangle {
                    id: stepCircleBigInside

                    anchors.centerIn: parent
                    radius: 4
                    width: 8
                    height: 8
                    color: stepCircleBig.color
                }
            }

            Texts.Text {
                id: stepText

                width: stepText.implicitWidth > (substepContent.width - 40) ? (substepContent.width - 40) : stepText.implicitWidth
                color: Styles.iconButton
                font.pixelSize: Texts.Text.Size.Small
                font.weight: Font.DemiBold
            }
        }
    }
}
