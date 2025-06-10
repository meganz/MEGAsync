import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.accountData 1.0
import components.pages 1.0

import SyncInfo 1.0

import syncs 1.0

FooterButtonsPage {
    id: root

    property alias localFolderChooser: localFolder
    property alias remoteFolderChooser: remoteFolder

    readonly property int columSpacing: 32

    footerButtons {
        rightPrimary {
            text: Strings.next
            icons.source: Images.arrowRight
        }

        rightSecondary {
            visible : syncsDataAccess.syncOrigin === SyncInfo.ONBOARDING_ORIGIN
        }
    }

    ColumnLayout {
        id: column

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: columSpacing

        HeaderTexts {
            id: header

            Layout.preferredWidth: parent.width
            title: SyncsStrings.selectiveSyncTitle
            description: SyncsStrings.selectiveSyncDescription
        }

        InfoAccount {
            id: accountData

            Layout.preferredWidth: parent.width
        }

        ColumnLayout {
            id: foldersColumn

            spacing: Constants.defaultComponentSpacing
            Layout.preferredWidth: parent.width

            ChooseSyncFolder {
                id: localFolder

                title: SyncsStrings.selectLocalFolder
                leftIconSource: Images.pc
                chosenPath: syncsDataAccess.defaultLocalFolder
            }

            ChooseSyncFolder {
                id: remoteFolder

                title: SyncsStrings.selectMEGAFolder
                leftIconSource: Images.megaOutline
                chosenPath: syncsDataAccess.defaultRemoteFolder
            }
        }
    }

}
