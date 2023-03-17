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
        Done = 2,
        DoneConfirm = 3,
        DoneLight = 4
    }

    /*
     * Properties
     */

    property string title: ""
    property int toState: InfoStep.ToStates.Disabled

    /*
     * Object properties
     */

    width: 126
    height: 24

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
        readonly property string stateDoneConfirm: "DONE_CONFIRM"
        readonly property string stateDoneLight: "DONE_LIGHT"

        property var statesMap: new Map([
            [InfoStep.ToStates.Disabled, stateDisabled],
            [InfoStep.ToStates.Current, stateCurrent],
            [InfoStep.ToStates.Done, stateDone],
            [InfoStep.ToStates.DoneConfirm, stateDoneConfirm],
            [InfoStep.ToStates.DoneLight, stateDoneLight]
        ])

        /*
         * Object properties
         */

        spacing: 8
        state: stateDisabled
        states: [
            State {
                name: mainLayout.stateDisabled
                PropertyChanges { target: circleBorder; color: Styles.buttonSecondaryPressed }
                PropertyChanges {
                    target: circleContent
                    width: 20
                    height: 20
                    radius: 20
                }
                PropertyChanges { target: circleInside; visible: true }
                PropertyChanges { target: stepTitle; color: Styles.buttonSecondaryPressed }
                PropertyChanges { target: checkImage; visible: false }
            },
            State {
                name: mainLayout.stateCurrent
                PropertyChanges { target: circleBorder; color: Styles.buttonPrimaryPressed }
                PropertyChanges {
                    target: circleContent
                    width: 8
                    height: 8
                    radius: 8
                }
                PropertyChanges { target: circleInside; visible: false }
                PropertyChanges { target: stepTitle; color: Styles.textPrimary }
                PropertyChanges { target: checkImage; visible: true }
            },
            State {
                name: mainLayout.stateDone
                PropertyChanges { target: circleBorder; visible: false }
                PropertyChanges { target: circleContent; visible: false }
                PropertyChanges { target: circleInside; visible: false }
                PropertyChanges { target: stepTitle; color: Styles.textSecondary }
                PropertyChanges {
                    target: checkImage
                    visible: true
                    source: "../../../../../images/Onboarding/check-circle_medium-regular-solid.svg"
                }
            },
            State {
                name: mainLayout.stateDoneConfirm
                PropertyChanges { target: circleBorder; color: Styles.buttonPrimaryPressed }
                PropertyChanges {
                    target: circleContent
                    visible: true
                    color: "#FFFFFF"
                    width: 18
                    height: 18
                }
                PropertyChanges {
                    target: circleInside
                    visible: true
                    color: Styles.buttonPrimaryPressed
                }
                PropertyChanges { target: stepTitle; color: Styles.textPrimary }
                PropertyChanges { target: checkImage; visible: false }
            },
            State {
                name: mainLayout.stateDoneLight
                PropertyChanges { target: circleBorder; visible: false }
                PropertyChanges { target: circleContent; visible: false }
                PropertyChanges { target: circleInside; visible: false }
                PropertyChanges { target: stepTitle; color: Styles.textPrimary }
                PropertyChanges {
                    target: checkImage;
                    visible: true
                    source: "../../../../../images/Onboarding/check-circle_medium-regular-outline.svg"
                }
            }
        ]

        Rectangle {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24

            Image {
                id: checkImage

                visible: false
            }

            Rectangle {
                id: circleBorder

                color: Styles.buttonSecondaryPressed
                width: 22
                height: 22
                radius: 22

                Rectangle {
                    id: circleContent

                    color: Styles.pageBackground
                    width: 8
                    height: 8
                    radius: 8
                    anchors.centerIn: parent

                    Rectangle {
                        id: circleInside

                        color: Styles.buttonSecondaryPressed
                        visible: false
                        width: 8
                        height: 8
                        radius: 8
                        anchors.centerIn: parent
                    }
                }
            }
        }

        Text {
            id: stepTitle

            text: title
            font.pixelSize: 12
            color: Styles.textPrimary
        }

    } // RowLayout -> mainLayout

} // Item
