import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.accountData 1.0
import components.pages 1.0
import components.texts 1.0

import SyncInfo 1.0
import ServiceUrls 1.0

Item {
    id: root

    readonly property int textSpacings: 8
    readonly property int foldersFolderSpacing: 12
    readonly property int localFolderChooserMinHeight: 82
    readonly property int remoteFolderChooserMinHeight: 98
    property alias footerButtons: footerButtonsItem
    property alias localFolderChooser: localFolder
    property alias remoteFolderChooser: remoteFolder
    property alias helpLink: helpLinkItem

    implicitHeight: layoutItem.height + Constants.defaultComponentSpacing

    ColumnLayout {
        id: layoutItem

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        ColumnLayout {
            id: textColumn

            spacing: root.textSpacings
            Layout.fillWidth: true

            HeaderTexts {
                id: header

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

        Item {
            Layout.preferredHeight: root.foldersFolderSpacing
        }

        ColumnLayout {
            id: foldersColumn

            Layout.preferredWidth: parent.width + 2 * Constants.focusBorderWidth

            ChooseSyncFolder {
                id: localFolder

                title: SyncsStrings.selectLocalFolder
                leftIconSource: Images.pc
                chosenPath: syncsDataAccess.defaultLocalFolder
                Layout.fillWidth: true
                Layout.leftMargin: -Constants.focusBorderWidth
                Layout.preferredHeight: Math.max(root.localFolderChooserMinHeight, folderField.height)
            }

            ChooseSyncFolder {
                id: remoteFolder

                title: SyncsStrings.selectMEGAFolder
                leftIconSource: Images.megaOutline
                chosenPath: syncsDataAccess.defaultRemoteFolder
                Layout.fillWidth: true
                Layout.leftMargin: -Constants.focusBorderWidth
                Layout.preferredHeight: Math.max(root.remoteFolderChooserMinHeight, folderField.height)
            }
        }

        Item { // trick: wrapper to avoid the anchoring colision (inside the footerbuttons) with the layout manager. that's the only purpose.
            Layout.fillWidth: true
            Layout.preferredHeight: footerButtonsItem.implicitHeight

            FooterButtons {
                id: footerButtonsItem

                anchors.bottomMargin: 0

                rightPrimary {
                    text: SyncsStrings.sync
                    icons.source: Images.syncIcon
                }

                rightSecondary {
                    text: syncsDataAccess.syncOrigin === SyncInfo.ONBOARDING_ORIGIN ? Strings.previous : Strings.cancel
                    visible : true
                }
            }
        }

        Item {
            Layout.preferredHeight: Constants.defaultComponentSpacing
        }
    }
}
