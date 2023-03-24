import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12

import Onboarding 1.0
import Onboard.Syncs_types 1.0

Item {
    id: rootItem

    StackView {
        id: stack

        anchors.fill: parent
        initialItem: welcomePage
    }

    Component {
        id: welcomePage

        Welcome {}
    }

    Component{
        id: loginPage

        RegisterFlow {}
    }

    Component {
        id: syncsFlow

        SyncsFlow {
            width: stack.width
            height: stack.height
        }
    }

    Connections{
        target: Onboarding

        onLoginFinished: {
            stack.replace(syncsFlow);
        }
    }
}
