import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.pages 1.0

FooterButtonsPage {
    id: root

    footerButtons {
        leftPrimary.visible: false
        rightSecondary {
            text: Strings.viewInSettings
            visible: backupCandidatesComponentAccess != null ? !backupCandidatesComponentAccess.comesFromSettings : false
        }
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
            topMargin: 24
        }
        spacing: 8

        Image {
            id: image

            source: Images.megaDevices
            sourceSize: Qt.size(120, 120)
        }

        Texts.Text {
            id: titleItem

            Layout.preferredWidth: parent.width
            Layout.topMargin: 40
            text: BackupsStrings.finalStepBackupTitle
            font {
                pixelSize: Texts.Text.Size.LARGE
                weight: Font.Bold
            }
            wrapMode: Text.Wrap
        }

        Texts.SecondaryText {
            id: descriptionItem

            Layout.preferredWidth: parent.width
            text: BackupsStrings.finalStepBackup
            font.pixelSize: Texts.Text.Size.MEDIUM
            wrapMode: Text.Wrap
            lineHeight: 20
            lineHeightMode: Text.FixedHeight
        }

        Texts.SecondaryText {
            id: descriptionItem2

            Layout.preferredWidth: parent.width
            text: BackupsStrings.finalStepBackup2
            font.pixelSize: Texts.Text.Size.MEDIUM
            wrapMode: Text.Wrap
            lineHeight: 20
            lineHeightMode: Text.FixedHeight
            visible: backupCandidatesComponentAccess != null ? !backupCandidatesComponentAccess.comesFromSettings : false
        }

    } // ColumnLayout: mainLayout

}
