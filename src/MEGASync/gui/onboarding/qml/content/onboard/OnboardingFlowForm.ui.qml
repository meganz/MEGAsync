import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12

import Onboarding 1.0
import Onboard.Syncs_types 1.0

Item {
    id: rootItem
    property alias welcomePage: welcomePage
    property alias registerFlow: registerFlow
    property alias syncsFlow: syncsFlow

    readonly property string welcome: "welcome"
    readonly property string register: "register"
    readonly property string syncs: "syncs"
    readonly property string syncSetupFinalize: "syncSetupFinalize"

    states: [
        State {
            name: syncs
            PropertyChanges {
                target: syncsFlow
                visible: true
            }
            PropertyChanges {
                target: welcomePage
                visible: false
            }
        },
        State {
            name: register
            PropertyChanges {
                target: registerFlow
                visible: true
            }
            PropertyChanges {
                target: welcomePage
                visible: false
            }
        },
        State {
            name: welcome
            PropertyChanges {
                target: welcomePage
                visible: true
            }
        }
    ]

    Welcome {
        id: welcomePage
        visible: true
    }

    RegisterFlow {
        id: registerFlow
        visible: false
    }

    SyncsFlow {
        id: syncsFlow
        visible: false
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

