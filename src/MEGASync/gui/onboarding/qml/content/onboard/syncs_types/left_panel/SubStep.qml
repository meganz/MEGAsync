// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts

Item {

    enum ToStates {
        Disabled = 0,
        Current = 1,
        Done = 2
    }

    property string title: ""
    property int toState: SubStep.ToStates.Disabled

    readonly property int diameter: 4
    readonly property int borderWidth: 8

    width: mainLayout.width
    height: mainLayout.height

    onToStateChanged: {
        mainLayout.state = mainLayout.statesMap.get(toState);
    }

    RowLayout {
        id: mainLayout

        readonly property string stateDisabled: "DISABLED"
        readonly property string stateCurrent: "CURRENT"
        readonly property string stateDone: "DONE"

        property var statesMap: new Map([
            [SubStep.ToStates.Disabled, stateDisabled],
            [SubStep.ToStates.Current, stateCurrent],
            [SubStep.ToStates.Done, stateDone]
        ])

        spacing: 14
        state: stateDisabled
        states: [
            State {
                name: mainLayout.stateDisabled
                PropertyChanges { target: circleBorder; color: Styles.buttonSecondaryPressed }
                PropertyChanges {
                    target: circleContent;
                    color: Styles.pageBackground
                    visible: true
                }
                PropertyChanges { target: stepTitle; color: Styles.buttonSecondaryPressed }
            },
            State {
                name: mainLayout.stateCurrent
                PropertyChanges { target: circleBorder; color: Styles.buttonPrimaryPressed }
                PropertyChanges { target: circleContent; visible: false }
                PropertyChanges { target: stepTitle; color: Styles.textSecondary }
            },
            State {
                name: mainLayout.stateDone
                PropertyChanges { target: circleBorder; color: Styles.buttonPrimaryPressed }
                PropertyChanges {
                    target: circleContent;
                    color: Styles.pageBackground
                    visible: true
                }
                PropertyChanges { target: stepTitle; color: Styles.textSecondary }
            }
        ]

        Rectangle {
            id: circleBorder

            color: Styles.buttonPrimaryPressed
            Layout.preferredWidth: borderWidth
            Layout.preferredHeight: borderWidth
            Layout.alignment: Qt.AlignVCenter
            radius: borderWidth

            Rectangle {
                id: circleContent

                color: Styles.pageBackground
                width: diameter
                height: diameter
                radius: diameter
                anchors.centerIn: parent
            }
        }

        MegaTexts.Text {
            id: stepTitle

            text: title
            font.pixelSize: MegaTexts.Text.Size.Small
            Layout.alignment: Qt.AlignLeft
            horizontalAlignment: Text.AlignLeft
        }
    }

}
