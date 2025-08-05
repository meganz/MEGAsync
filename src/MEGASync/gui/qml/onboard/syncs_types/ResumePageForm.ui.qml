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

    readonly property int buttonQuestionMargin: 24
    readonly property int topMargin: 8
    readonly property int topMainQuestionMarging: 32
    readonly property int topButtonGroupMargin: 16

    property alias buttonGroup: buttonGroupItem
    property alias syncButton: syncButtonItem
    property alias titleItem: titleItem
    property alias descriptionItem: descriptionItem
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
        spacing: 0

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
            Layout.topMargin: topMargin
            font.pixelSize: Texts.Text.Size.MEDIUM
            wrapMode: Text.Wrap
            text: SyncsStrings.finalStepSync
        }

        Texts.SecondaryText {
            id: descriptionItem2

            Layout.topMargin: topMargin
            Layout.preferredWidth: parent.width
            font.pixelSize: Texts.Text.Size.MEDIUM
            wrapMode: Text.Wrap
        }

        Texts.Text {
            id: finalStepQuestionText

            Layout.preferredWidth: parent.width
            Layout.topMargin: topMainQuestionMarging
            text: OnboardingStrings.finalStepQuestion
            font {
                pixelSize: Texts.Text.Size.MEDIUM_LARGE
                weight: Font.DemiBold
            }
        }

        Item {
            id: buttons

            Layout.preferredWidth: parent.width + (2 * Constants.focusBorderWidth)
            Layout.topMargin: topButtonGroupMargin

            ButtonGroup {
                id: buttonGroupItem
            }

            RowLayout {
                id: buttonsLayout

                anchors {
                    fill: parent
                    leftMargin: -Constants.focusBorderWidth
                    rightMargin: Constants.focusBorderWidth
                }
                spacing: 12

                SyncsVerticalButton {
                    id: syncButtonItem

                    Layout.fillWidth: true
                    title: SyncsStrings.sync
                    description: OnboardingStrings.finalPageButtonSelectiveSync
                    imageSource: Images.sync
                    focus: true
                    useMaxSiblingHeight: true
                    ButtonGroup.group: buttonGroupItem
                }

                SyncsVerticalButton {
                    id: backupsButton

                    Layout.fillWidth: true
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
