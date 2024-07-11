import QtQuick 2.0
import components.texts 1.0 as Texts
import components.buttons 1.0 as Buttons
import components.views 1.0

Item {
    id: root

    readonly property string noConnectionsState: "noConnectionsState"
    readonly property string noInternetState: "noInternetState"
    readonly property string syncTableState: "syncTableState"

    StackViewBase{
        id: stackView

        anchors.fill: parent
        state: syncTableState
        states: [
            State {
                name: noInternetState
                StateChangeScript {
                    script: stackView.replace(noInternetComponent);
                }
            },
            State {
                name: noConnectionsState
                StateChangeScript {
                    script: stackView.replace(noConnectionsComponent);
                }
            },
            State {
                name: syncTableState
                StateChangeScript {
                    script: stackView.replace(syncTableComponent);
                }
            }
        ] // States

        Component {
            id: noInternetComponent

            NoInternetPage {}
        }

        Component {
            id: noConnectionsComponent

            NoConnectionsPage {}
        }

        Component {
            id: syncTableComponent

            SyncTablePage {}
        }
    }
}
