import QtQuick 2.15

import common 1.0

import components.views 1.0

Item {
    id: root

    required property Component addSyncPageComponent
    required property Component confirmSyncsPageComponent

    readonly property string addSync: "addSync"
    readonly property string confirmSyncs: "confirmSyncs"

    // added to avoid qml warning.
    function setInitialFocusPosition() { }

    state: root.addSync

    states: [
        State {
            name: root.addSync
            StateChangeScript {
                script: {
                    view.replace(addSyncPageComponent);
                }
            }
        },
        State {
            name: root.confirmSyncs
            StateChangeScript {
                script: {
                    view.replace(confirmSyncsPageComponent);
                }
            }
        }
    ]

    StackViewBase {
        id: view

        anchors.fill: parent
        onCurrentItemChanged: {
            currentItem.setInitialFocusPosition();
        }
    }
}
