import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.accountData 1.0
import components.pages 1.0
import components.texts 1.0

import SyncInfo 1.0
import ServiceUrls 1.0

FooterButtonsPage {
    id: root

    property alias localFolderChooser: localFolder
    property alias remoteFolderChooser: remoteFolder
    property alias helpLink: helpLinkItem

    readonly property int textSpacings: 8

    footerButtons {
        rightPrimary {
            text: SyncsStrings.sync
            icons.source: Images.syncIcon
        }

        rightSecondary {
            text: syncsDataAccess.syncOrigin === SyncInfo.ONBOARDING_ORIGIN ? Strings.previous : Strings.cancel
            visible : true
        }
    }

    ColumnLayout {
        id: column

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 0
        }

        spacing: Constants.defaultComponentSpacing
                 - (localFolder.folderField.hint.visible + remoteFolder.folderField.hint.visible)
                    * Constants.defaultComponentSpacing / 3


        ColumnLayout {
            Layout.fillWidth: true
            spacing: (localFolder.folderField.hint.visible && remoteFolder.folderField.hint.visible) ?
                         textSpacings / 4
                       : textSpacings
            HeaderTexts {
                id: header

                Layout.fillWidth: true
                title: SyncsStrings.selectiveSyncTitle
                description: SyncsStrings.selectiveSyncDescription
            }

            RichText {
                id: helpLinkItem

                manageMouse: true
                manageHover: true
                underlineLink: true
                rawText: SyncsStrings.helpSync
                font.pixelSize: Text.Size.MEDIUM
                visible: syncsDataAccess.syncOrigin !== SyncInfo.ONBOARDING_ORIGIN
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Constants.defaultComponentSpacing
                     - (localFolder.folderField.hint.visible + remoteFolder.folderField.hint.visible)
                        * Constants.defaultComponentSpacing / 2.5

            ChooseSyncFolder {
                id: localFolder

                title: SyncsStrings.selectLocalFolder
                leftIconSource: Images.pc
                chosenPath: syncsDataAccess.defaultLocalFolder
                Layout.fillWidth: true
            }

            ChooseSyncFolder {
                id: remoteFolder

                title: SyncsStrings.selectMEGAFolder
                leftIconSource: Images.megaOutline
                chosenPath: syncsDataAccess.defaultRemoteFolder
                Layout.fillWidth: true
            }
        }
    }

}
