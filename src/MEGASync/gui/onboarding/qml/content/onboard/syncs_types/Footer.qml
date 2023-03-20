import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Components 1.0 as Custom

RowLayout {
    id: mainLayout

    /*
     * Functions
     */

    function show(footerState) {
        var footerStateStr = statesMap.get(footerState);
        if(footerStateStr === undefined) {
            console.error("Footer: Undefined state -> " + footerState);
        } else {
            currentFooterState = footerState;
            mainLayout.state = footerStateStr;
        }
    }

    /*
     * Enums
     */

    enum ToStates {
        CancelNext = 0,
        CancelPreviousNextDisabled = 1,
        CancelPreviousNextEnabled = 2,
        CancelPreviousNextBackup = 3
    }

    /*
     * Properties
     */

    readonly property string stateCancelNext: "CANCEL_NEXT"
    readonly property string stateCancelPreviousNextDisabled: "CANCEL_PREV_NEXT_DIS"
    readonly property string stateCancelPreviousNextEnabled: "CANCEL_PREV_NEXT_ENA"
    readonly property string stateCancelPreviousNextBackup: "CANCEL_PREV_NEXT_BACKUP"

    property var statesMap: new Map([
        [Footer.ToStates.CancelNext, stateCancelNext],
        [Footer.ToStates.CancelPreviousNextDisabled, stateCancelPreviousNextDisabled],
        [Footer.ToStates.CancelPreviousNextEnabled, stateCancelPreviousNextEnabled],
        [Footer.ToStates.CancelPreviousNextBackup, stateCancelPreviousNextBackup]
    ])

    property int currentFooterState: Footer.ToStates.CancelNext
    property int lastNextState: Footer.ToStates.CancelPreviousNextDisabled

    /*
     * Signals
     */

    signal cancelButtonClicked
    signal nextButtonClicked
    signal previousButtonClicked

    /*
     * Object properties
     */

    state: stateCancelNext
    states: [
        State {
            name: mainLayout.stateCancelNext
            PropertyChanges { target: previousButton; visible: false }
            PropertyChanges {
                target: nextButton;
                enabled: true
                iconSource: "../../../images/Onboarding/arrow_right.svg"
            }
        },
        State {
            name: mainLayout.stateCancelPreviousNextDisabled
            PropertyChanges { target: previousButton; visible: true }
            PropertyChanges {
                target: nextButton;
                enabled: false
                iconSource: "../../../images/Onboarding/arrow_right.svg"
            }
        },
        State {
            name: mainLayout.stateCancelPreviousNextEnabled
            PropertyChanges { target: previousButton; visible: true }
            PropertyChanges {
                target: nextButton;
                enabled: true
                iconSource: "../../../images/Onboarding/arrow_right.svg"
            }
        },
        State {
            name: mainLayout.stateCancelPreviousNextBackup
            PropertyChanges { target: previousButton; visible: true }
            PropertyChanges {
                target: nextButton;
                enabled: true
                iconSource: "../../../images/Onboarding/cloud.svg"
                sourceSize: Qt.size(25, 25)
                text: qsTr("Backup")
            }
        }
    ]

    /*
     * Child objects
     */

    Custom.Button {
        id: cancelButton

        text: qsTr("Cancel")
        onClicked: {
            cancelButtonClicked();
        }
    }

    Custom.Button {
        id: previousButton

        text: qsTr("Previous")
        onClicked: {
            previousButtonClicked();
        }
    }

    Custom.Button {
        id: nextButton

        text: qsTr("Next")
        primary: true
        iconRight: true
        iconSource: "../../../images/Onboarding/arrow_right.svg"
        onClicked: {
            nextButtonClicked();
        }
    }
}
