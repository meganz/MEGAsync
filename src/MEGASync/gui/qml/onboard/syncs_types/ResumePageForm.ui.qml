import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.buttons 1.0
import components.pages 1.0

import syncs 1.0

import onboard 1.0

FooterButtonsPage {
    id: root

    readonly property int maxSizeDescription: 80
    readonly property int buttonQuestionMargin: 24

    property alias buttonGroup: buttonGroupItem
    property alias syncButton: syncButtonItem
    property alias titleItem: titleItem
    property alias descriptionItem: descriptionItem
    property alias errorItem: errorItem
    property alias descriptionItem2: descriptionItem2

    footerButtons {
        leftPrimary.visible: false
        rightSecondary.text: Strings.viewInSettings
        rightPrimary {
            text: Strings.done
            icons: Icon {}
        }
    }

    ColumnLayout {
        id: mainLayout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        Texts.Text {
            id: titleItem

            Layout.preferredWidth: parent.width
            font {
                pixelSize: Texts.Text.Size.LARGE
                weight: Font.Bold
            }
            wrapMode: Text.Wrap
        }

        Texts.SecondaryText {
            id: descriptionItem

            Layout.preferredWidth: parent.width
            Layout.topMargin: 8
            font.pixelSize: Texts.Text.Size.MEDIUM
            wrapMode: Text.Wrap
            text: SyncsStrings.finalStepSync
        }

        Texts.BannerText {
            id: errorItem

            showBorder: false
            type: Constants.MessageType.ERROR
            text: OnboardingStrings.finalStepSyncError
            visible: false
            backgroundColor: ColorTheme.notificationError
            icon: Images.xCircle
            Layout.preferredWidth: parent.width
            Layout.topMargin: 8
            Layout.bottomMargin: 15
        }

        Texts.SecondaryText {
            id: descriptionItem2

            Layout.preferredWidth: parent.width
            font.pixelSize: Texts.Text.Size.MEDIUM
            wrapMode: Text.Wrap
        }

        Texts.Text {
            id: finalStepQuestionText

            Layout.preferredWidth: parent.width
            Layout.topMargin: (descriptionItem.height > maxSizeDescription) ? (buttonQuestionMargin * 0.5) : buttonQuestionMargin
            text: OnboardingStrings.finalStepQuestion
            font {
                pixelSize: Texts.Text.Size.MEDIUM_LARGE
                weight: Font.DemiBold
            }
        }

        Item {
            id: buttons

            Layout.preferredWidth: parent.width + 8
            Layout.topMargin: (descriptionItem.height > maxSizeDescription) ? (buttonQuestionMargin * 0.5) : buttonQuestionMargin

            ButtonGroup {
                id: buttonGroupItem
            }

            RowLayout {
                id: buttonsLayout

                anchors {
                    fill: parent
                    leftMargin: -syncButtonItem.focusBorderWidth
                    rightMargin: backupsButton.focusBorderWidth
                }
                spacing: 12

                SyncsVerticalButton {
                    id: syncButtonItem

                    width: (parent.width - parent.spacing) / 2
                    title: SyncsStrings.sync
                    description: OnboardingStrings.finalPageButtonSelectiveSync
                    imageSource: Images.sync
                    focus: true
                    useMaxSiblingHeight: true
                    ButtonGroup.group: buttonGroupItem
                }

                SyncsVerticalButton {
                    id: backupsButton

                    width: !syncButtonItem.visible
                           ? parent.width
                           : (parent.width - parent.spacing) / 2
                    title: OnboardingStrings.backup
                    description: OnboardingStrings.finalPageButtonBackup
                    imageSource: Images.installationTypeBackups
                    type: Constants.SyncType.BACKUP
                    useMaxSiblingHeight: syncButtonItem.visible
                    ButtonGroup.group: buttonGroupItem
                }
            }
        }
    }

}
