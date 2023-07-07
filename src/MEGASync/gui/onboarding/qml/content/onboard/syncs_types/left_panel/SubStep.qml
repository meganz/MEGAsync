// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts

Item {
    id: root

    enum ToStates {
        Disabled = 0,
        Current = 1,
        Done = 2,
        Warning = 3,
        Error = 4
    }

    property string text: ""
    property int toState: SubStep.ToStates.Disabled

    onToStateChanged: {
        substepContent.state = substepContent.statesMap.get(toState);
    }

    height: substepContent.height
    width: substepContent.width
    Layout.preferredWidth: substepContent.width
    Layout.preferredHeight: substepContent.height

    Rectangle {
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

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 5
            anchors.topMargin: 4
            anchors.bottomMargin: 4
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
            }

            Item {
                id: outlinedCircle

                width: 14
                height: 14

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

            MegaTexts.Text {
                id: stepText

                text: root.text
                color: Styles.iconButton
                font.pixelSize: MegaTexts.Text.Size.Small
                font.weight: Font.DemiBold
            }
        }
    }

}
