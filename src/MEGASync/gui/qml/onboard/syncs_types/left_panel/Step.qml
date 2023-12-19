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
        DONE = 3
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

        property var statesMap: new Map([
            [Step.ToStates.DISABLED, stateDisabled],
            [Step.ToStates.CURRENT, stateCurrent],
            [Step.ToStates.CURRENT_SUBSTEP, stateCurrentSubstep],
            [Step.ToStates.DONE, stateDone]
        ])

        width: content.width + 17
        height: 32
        Layout.preferredWidth: width
        Layout.preferredHeight: height
        radius: 16
        color: Styles.iconButtonPressedBackground

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
                PropertyChanges { target: stepCircle; color: Styles.iconButton;  border.width: 0; }
                PropertyChanges { target: stepCircleImage; visible: false; }
                PropertyChanges { target: stepCircleText; color: Styles.textInverseAccent; }
                PropertyChanges { target: stepText; color: Styles.iconButton; }
            },
            State {
                name: stepContent.stateCurrentSubstep
                PropertyChanges { target: stepContent; color: "transparent"; }
                PropertyChanges { target: stepCircle; color: Styles.iconButton; border.width: 0; }
                PropertyChanges { target: stepCircleImage; visible: false; }
                PropertyChanges { target: stepCircleText; color: Styles.textInverseAccent; }
                PropertyChanges { target: stepText; color: Styles.iconButton; }
            },
            State {
                name: stepContent.stateDone
                PropertyChanges { target: stepContent; color: "transparent"; }
                PropertyChanges { target: stepCircle; color: Styles.supportSuccess; border.width: 0; }
                PropertyChanges { target: stepCircleImage; visible: true; }
                PropertyChanges { target: stepCircleText; color: Styles.textInverseAccent; visible: false; }
                PropertyChanges { target: stepText; color: Styles.iconButton; }
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
                color: Styles.iconButton

                Texts.Text {
                    id: stepCircleText

                    anchors.centerIn: parent
                    text: number.toString()
                    color: Styles.textInverseAccent
                    font.bold: true
                    lineHeight: 16
                    lineHeightMode: Text.FixedHeight
                }

                SvgImage {
                    id: stepCircleImage

                    anchors.centerIn: parent
                    source: Images.check
                    sourceSize: Qt.size(2 * parent.width / 3, 2 * parent.width / 3)
                    color: Styles.textInverseAccent
                    visible: false
                }
            }

            Texts.Text {
                id: stepText

                color: Styles.iconButton
                font.weight: Font.DemiBold
            }

        } // RowLayout: content

    } // Rectangle: stepContent

}
