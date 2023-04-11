import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Onboard.Syncs_types 1.0
import Components 1.0 as Custom
import Common 1.0
import BackupFolderModel 1.0

SyncsPage {

    property alias mainLayout: mainLayout

    property FoldersTable backupTable

    ColumnLayout {
        spacing: 24
        anchors.left: parent.left
        anchors.leftMargin: 32
        width: 488

        Header {
            title: qsTr("Confirm folders to back up")
            description: qsTr("Will be added to your cloud and available in your other devices.")
            Layout.fillWidth: false
            Layout.preferredWidth: parent.width
            Layout.topMargin: 32
        }

        ColumnLayout {
            id: mainLayout

            width: parent.width
            spacing: 24

            FoldersTable {
                backupProxyModel: backupTable.backupProxyModel
                backupModel: backupTable.backupModel
            }

            ColumnLayout {
                spacing: 4

                Text {
                    text: qsTr("Backup to:")
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                    font.family: "Inter"
                    font.styleName: "normal"
                }

                Rectangle {
                    Layout.preferredWidth: mainLayout.width
                    Layout.preferredHeight: 36
                    color: Styles.pageBackground
                    border.color: Styles.borderStrong
                    border.width: 2
                    radius: 8

                    RowLayout {
                        id: backupToInfo

                        spacing: 6
                        anchors.verticalCenter: parent.verticalCenter

                        Custom.SvgImage {
                            Layout.leftMargin: 10
                            source: "../../../../../../images/Onboarding/cloud.svg"
                            sourceSize: Qt.size(16, 16)
                            color: Styles.iconSecondary
                        }

                        Text {
                            text: qsTr("/Backups")
                            color: Styles.textPlaceholder
                            font.family: "Inter"
                            font.styleName: "normal"
                            font.weight: Font.Thin
                            font.pixelSize: 14
                        }
                    }
                }
            }
        }
    }

}
