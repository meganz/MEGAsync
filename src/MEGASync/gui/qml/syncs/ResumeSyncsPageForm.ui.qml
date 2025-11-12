import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.pages 1.0

import SyncInfo 1.0

FooterButtonsPage {
    id: root

    property alias image : imageItem

    footerButtons {
        leftSecondary.visible: false
        rightSecondary {
            text: Strings.viewInSettings
            visible: syncsDataAccess.syncOrigin  !== SyncInfo.MAIN_APP_ORIGIN
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
        spacing: 8

        Image {
            id: imageItem

            source: Images.megaDevices
            sourceSize: Qt.size(120, 120)
        }

        Texts.Text {
            id: titleItem

            Layout.preferredWidth: parent.width
            Layout.topMargin: 40
            text: SyncsStrings.finalStepSyncTitle
            font {
                pixelSize: Texts.Text.Size.LARGE
                weight: Font.Bold
            }
            wrapMode: Text.Wrap
        }

        Texts.SecondaryText {
            id: descriptionItem

            Layout.topMargin: 30
            Layout.preferredWidth: parent.width
            text: SyncsStrings.finalStepSync
            font.pixelSize: Texts.Text.Size.MEDIUM
            wrapMode: Text.Wrap
            lineHeight: 20
            lineHeightMode: Text.FixedHeight
        }

    } // ColumnLayout: mainLayout

}
