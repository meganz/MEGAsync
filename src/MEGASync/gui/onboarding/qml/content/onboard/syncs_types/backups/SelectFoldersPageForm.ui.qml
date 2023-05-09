// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0
import Components 1.0 as Custom

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

// C++
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

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 32
        }
        spacing: 24

        Header {
            title: OnboardingStrings.selectBackupFoldersTitle
            description: OnboardingStrings.selectBackupFoldersDescription
        }

        ColumnLayout {

            Layout.preferredWidth: parent.width
            spacing: 12

            InfoAccount {
                Layout.preferredWidth: parent.width
            }

            FoldersTable {
                id: backupTable

                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 186
                backupProxyModel: backupProxyModel
                backupModel: backupModel
            }

            Rectangle {
                Layout.preferredWidth: parent.width

                RowLayout {
                    id: addFolders

                    spacing: 9

                    Custom.SvgImage {
                        Layout.leftMargin: 19
                        source: Images.plusCircle
                        color: Styles.buttonPrimary
                        sourceSize: Qt.size(22, 22)
                    }

                    Text {
                        text: OnboardingStrings.addFolders
                        font.family: "Inter"
                        font.styleName: "normal"
                        font.weight: Font.DemiBold
                        font.pixelSize: 12
                        font.underline: true
                        color: Styles.buttonPrimary
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
