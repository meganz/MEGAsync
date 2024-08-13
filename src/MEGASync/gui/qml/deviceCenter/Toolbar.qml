import QtQuick 2.0
import QtQuick.Layouts 1.15

import common 1.0
import components.buttons 1.0
import components.images 1.0

Item {
    id:root

    Rectangle {
        id: contentItem

        anchors.fill: parent

        RowLayout {
            id: layout

            spacing: Constants.defaultComponentSpacing - 8
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
                rightMargin: Constants.defaultComponentSpacing - addSync.sizes.focusBorderWidth
            }

            ToolbarButton {
                id: addBackup

                text: DeviceCenterStrings.addBackupLabel
                icons.source: Images.addBackup

                onClicked: {
                    let fromSettings = true;
                    deviceCenterAccess.openAddBackupDialog(fromSettings);
                }
            }

            ToolbarButton {
                id: addSync

                text: DeviceCenterStrings.addSyncLabel
                icons.source: Images.addSync

                onClicked: {
                    deviceCenterAccess.openAddSyncDialog();
                }
            }
        }
    }
}
