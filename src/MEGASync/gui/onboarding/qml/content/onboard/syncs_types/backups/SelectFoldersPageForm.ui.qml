// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

SyncsPage {

    property alias addFoldersMouseArea: addFoldersMouseArea

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
                model: _cppBackupsModel
            }

            Rectangle {
                Layout.preferredWidth: parent.width

                RowLayout {
                    id: addFolders

                    spacing: 9

                    MegaImages.SvgImage {
                        Layout.leftMargin: 19
                        source: Images.plusCircle
                        color: Styles.buttonPrimary
                        sourceSize: Qt.size(22, 22)
                    }

                    MegaTexts.Text {
                        text: OnboardingStrings.addFolders
                        font.weight: Font.DemiBold
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
