import QtQuick 2.12
import QtQuick.Layouts 1.12
import Common 1.0

Item {

    /*
     * Enums
     */

    enum ToStates {
        Disabled = 0,
        Current = 1,
        Done = 2
    }

    /*
     * Properties
     */

    property string title: ""
    property int toState: SyncsInfoSubStep.ToStates.Disabled

    /*
     * Object properties
     */

    width: 126
    height: 16

    onToStateChanged: {
        mainLayout.state = mainLayout.statesMap.get(toState);
    }

    /*
     * Child objects
     */

    RowLayout {
        id: mainLayout

        /*
         * Properties
         */
        readonly property string stateDisabled: "DISABLED"
        readonly property string stateCurrent: "CURRENT"
        readonly property string stateDone: "DONE"

        property var statesMap: new Map([
            [SyncsInfoSubStep.ToStates.Disabled, stateDisabled],
            [SyncsInfoSubStep.ToStates.Current, stateCurrent],
            [SyncsInfoSubStep.ToStates.Done, stateDone]
        ])

        /*
         * Object properties
         */

        spacing: 16
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

        /*
         * Child objects
         */

        Rectangle {
            id: circleBorder

            color: Styles.buttonPrimaryPressed
            Layout.preferredWidth: 8
            Layout.preferredHeight: 8
            Layout.alignment: Qt.AlignVCenter
            radius: 8

            Rectangle {
                id: circleContent

                color: Styles.pageBackground
                width: 4
                height: 4
                radius: 4
                anchors.centerIn: parent
            }
        }

        Text {
            id: stepTitle

            text: title
            font.pixelSize: 10
            Layout.alignment: Qt.AlignLeft
            horizontalAlignment: Text.AlignLeft
            color: Styles.textPrimary
        }
    }
}
