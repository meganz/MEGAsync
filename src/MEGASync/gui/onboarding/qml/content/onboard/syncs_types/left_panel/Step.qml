// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components 1.0 as Custom

Item {

    enum ToStates {
        Disabled = 0,
        Current = 1,
        Done = 2,
        DoneConfirm = 3,
        DoneLight = 4
    }

    property string title: ""
    property int toState: Step.ToStates.Disabled

    readonly property int diameter: 22
    readonly property int borderWidth: 8
    readonly property int contentWidth: 18
    readonly property size doneImageSize: Qt.size(10, 7.5)

    width: mainLayout.width
    height: diameter

    onToStateChanged: {
        mainLayout.state = mainLayout.statesMap.get(toState);
    }

    RowLayout {
        id: mainLayout

        readonly property string stateDisabled: "DISABLED"
        readonly property string stateCurrent: "CURRENT"
        readonly property string stateDone: "DONE"
        readonly property string stateDoneConfirm: "DONE_CONFIRM"
        readonly property string stateDoneLight: "DONE_LIGHT"

        property var statesMap: new Map([
            [Step.ToStates.Disabled, stateDisabled],
            [Step.ToStates.Current, stateCurrent],
            [Step.ToStates.Done, stateDone],
            [Step.ToStates.DoneConfirm, stateDoneConfirm],
            [Step.ToStates.DoneLight, stateDoneLight]
        ])

        spacing: borderWidth
        state: stateDisabled
        states: [
            State {
                name: mainLayout.stateDisabled
                PropertyChanges { target: circleBorder; color: Styles.buttonSecondaryPressed }
                PropertyChanges {
                    target: circleContent
                    width: contentWidth
                    height: contentWidth
                    radius: contentWidth
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
                    width: borderWidth
                    height: borderWidth
                    radius: borderWidth
                }
                PropertyChanges { target: circleInside; visible: false }
                PropertyChanges { target: stepTitle; color: Styles.textPrimary }
                PropertyChanges { target: checkImage; visible: false }
            },
            State {
                name: mainLayout.stateDone
                PropertyChanges { target: circleBorder; color: Styles.buttonPrimaryPressed }
                PropertyChanges { target: circleContent; visible: false }
                PropertyChanges { target: circleInside; visible: false }
                PropertyChanges { target: stepTitle; color: Styles.textSecondary }
                PropertyChanges {
                    target: checkImage
                    visible: true
                    color: Styles.pageBackground
                }
            },
            State {
                name: mainLayout.stateDoneConfirm
                PropertyChanges { target: circleBorder; color: Styles.buttonPrimaryPressed }
                PropertyChanges {
                    target: circleContent
                    width: contentWidth
                    height: contentWidth
                    radius: contentWidth
                    visible: true;
                    color: Styles.pageBackground
                }
                PropertyChanges { target: circleInside; visible: true; color: Styles.buttonPrimaryPressed }
                PropertyChanges { target: stepTitle; color: Styles.textSecondary }
                PropertyChanges { target: checkImage; visible: false }
            },
            State {
                name: mainLayout.stateDoneLight
                PropertyChanges { target: circleBorder; color: Styles.buttonPrimaryPressed; }
                PropertyChanges {
                    target: circleContent
                    width: contentWidth
                    height: contentWidth
                    radius: contentWidth
                    visible: true;
                    color: Styles.pageBackground
                }
                PropertyChanges { target: circleInside; visible: false }
                PropertyChanges { target: stepTitle; color: Styles.textSecondary }
                PropertyChanges {
                    target: checkImage
                    visible: true
                    color: Styles.buttonPrimaryPressed
                }
            }
        ]

        Rectangle {
            id: circleBorder

            color: Styles.buttonSecondaryPressed
            Layout.preferredWidth: diameter
            Layout.preferredHeight: diameter
            radius: diameter

            Rectangle {
                id: circleContent

                color: Styles.pageBackground
                width: borderWidth
                height: borderWidth
                radius: borderWidth
                anchors.centerIn: parent

                Rectangle {
                    id: circleInside

                    color: Styles.buttonSecondaryPressed
                    visible: false
                    width: borderWidth
                    height: borderWidth
                    radius: borderWidth
                    anchors.centerIn: parent
                }
            }

            Custom.SvgImage {
                id: checkImage

                visible: false
                anchors.centerIn: parent
                sourceSize: doneImageSize
                source: Images.check
                color: Styles.pageBackground
            }
        }

        Custom.Text {
            id: stepTitle

            text: title
        }

    }

}
