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
        leftSecondary.visible: false
        rightSecondary {
            text: Strings.viewInSettings
            visible: backupsAccess != null ? !backupsAccess.comesFromSettings : false
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
        }

        Texts.Text {
            id: titleItem

            Layout.preferredWidth: parent.width
            text: BackupsStrings.finalStepBackupTitle
            font {
                pixelSize: Texts.Text.Size.LARGE
                weight: Font.Bold
            }
            wrapMode: Text.Wrap
        }

        Texts.Text {
            id: descriptionItem

            Layout.preferredWidth: parent.width
            Layout.topMargin: 8
            text: BackupsStrings.finalStepBackup
            font.pixelSize: Texts.Text.Size.MEDIUM
            wrapMode: Text.Wrap
        }

        Image {
            id: image

            Layout.topMargin: 48
            source: Images.okIcon
            sourceSize: Qt.size(120, 120)
        }

    } // ColumnLayout: mainLayout

}
