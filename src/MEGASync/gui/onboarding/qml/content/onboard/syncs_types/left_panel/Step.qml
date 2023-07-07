// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

Item {
    id: root

    enum ToStates {
        Disabled = 0,
        Current = 1,
        CurrentSubstep = 2,
        Done = 3
    }

    property int number: 0
    property string text: ""
    property int toState: Step.ToStates.Disabled

    onToStateChanged: {
        stepContent.state = stepContent.statesMap.get(toState);
    }

    height: stepContent.height
    width: stepContent.width
    Layout.preferredWidth: stepContent.width
    Layout.preferredHeight: stepContent.height

    Rectangle {
        id: stepContent

        readonly property string stateDisabled: "DISABLED"
        readonly property string stateCurrent: "CURRENT"
        readonly property string stateCurrentSubstep: "SUBSTEP"
        readonly property string stateDone: "DONE"

        property var statesMap: new Map([
            [Step.ToStates.Disabled, stateDisabled],
            [Step.ToStates.Current, stateCurrent],
            [Step.ToStates.CurrentSubstep, stateCurrentSubstep],
            [Step.ToStates.Done, stateDone]
        ])

        state: stateDisabled
        states: [
            State {
                name: stepContent.stateDisabled
                PropertyChanges { target: stepContent; color: "transparent"; }
                PropertyChanges {
                    target: stepCircle;
                    color: "transparent";
                    border.width: 2;
                    border.color: Styles.iconButtonDisabled;
                }
                PropertyChanges { target: stepCircleImage; visible: false; }
                PropertyChanges { target: stepCircleText; color: Styles.iconButtonDisabled; }
                PropertyChanges { target: stepText; color: Styles.iconButtonDisabled; }
            },
            State {
                name: stepContent.stateCurrent
                PropertyChanges { target: stepContent; color: Styles.iconButtonPressedBackground; }
                PropertyChanges {
                    target: stepCircle;
                    color: Styles.iconButton;
                    border.width: 0;
                }
                PropertyChanges { target: stepCircleImage; visible: false; }
                PropertyChanges { target: stepCircleText; color: "white"/*Styles.textInverseAccent;*/ }
                PropertyChanges { target: stepText; color: Styles.iconButton; }
            },
            State {
                name: stepContent.stateCurrentSubstep
                PropertyChanges { target: stepContent; color: "transparent"; }
                PropertyChanges {
                    target: stepCircle;
                    color: Styles.iconButton;
                    border.width: 0;
                }
                PropertyChanges { target: stepCircleImage; visible: false; }
                PropertyChanges { target: stepCircleText; color: Styles.textInverseAccent; }
                PropertyChanges { target: stepText; color: Styles.iconButton; }
            },
            State {
                name: stepContent.stateDone
                PropertyChanges { target: stepContent; color: "transparent"; }
                PropertyChanges {
                    target: stepCircle;
                    color: Styles.supportSuccess;
                    border.width: 0;
                }
                PropertyChanges { target: stepCircleImage; visible: true; }
                PropertyChanges {
                    target: stepCircleText;
                    color: Styles.textInverseAccent;
                    visible: false;
                }
                PropertyChanges { target: stepText; color: Styles.iconButton; }
            }
        ]

        radius: 16
        color: Styles.iconButtonPressedBackground
        height: 32
        width: content.width + 17
        Layout.preferredWidth: width
        Layout.preferredHeight: height

        RowLayout {
            id: content

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 5
            anchors.topMargin: 5
            anchors.bottomMargin: 5
            anchors.rightMargin: 12
            spacing: 8

            Rectangle {
                id: stepCircle

                radius: width / 2
                width: 22
                height: width
                color: Styles.iconButton

                MegaTexts.Text {
                    id: stepCircleText

                    anchors.centerIn: parent
                    text: number.toString()
                    color: Styles.textInverseAccent
                    font.bold: true
                }

                MegaImages.SvgImage {
                    id: stepCircleImage

                    anchors.centerIn: parent
                    source: Images.check
                    sourceSize: Qt.size(10, 7.5)
                    color: Styles.textInverseAccent
                    visible: false
                }
            }

            MegaTexts.Text {
                id: stepText

                text: root.text
                color: Styles.iconButton
                font.weight: Font.DemiBold
            }
        }
    }

}
