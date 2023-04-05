import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Components 1.0 as Custom
import Onboard.Syncs_types 1.0
import Common 1.0
import BackupFolderModel 1.0
import BackupFolderFilterProxyModel 1.0

SyncsPage {

    property alias addFoldersMouseArea: addFoldersMouseArea
    property alias backupTable: backupTable

    BackupFolderModel {
        id: backupModel
    }

    BackupFolderFilterProxyModel {
        id: backupProxyModel

        sourceModel: backupModel
    }

    ColumnLayout {
        spacing: 34
        anchors.left: parent.left
        anchors.leftMargin: 32
        width: 488

        Header {
            title: qsTr("Select folders to back up")
            description: qsTr("Selected folders from your computer to MEGA. Files will automatically back up when the desktop application is running.")
            Layout.fillWidth: false
            Layout.preferredWidth: parent.width
            Layout.topMargin: 32
        }

        ColumnLayout {
            width: parent.width

            InfoAccount {
            }

            FoldersTable {
                id: backupTable

                backupProxyModel: backupProxyModel
                backupModel: backupModel
            }

            Rectangle {
                Layout.topMargin: 12

                RowLayout {
                    id: addFolders
                    spacing: 9

                    Custom.SvgImage {
                        Layout.leftMargin: 19
                        source: "../../../../../../images/Onboarding/plus-circle.svg"
                        color: Styles.buttonPrimary
                    }

                    Text {
                        text: qsTr("Add folders")
                        font.family: "Inter"
                        font.styleName: "normal"
                        font.weight: Font.DemiBold
                        font.pixelSize: 12
                        font.underline: true
                    }
                }

                MouseArea {
                    id: addFoldersMouseArea

                    anchors.fill: addFolders
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }
    }
}
