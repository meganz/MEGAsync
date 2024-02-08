import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.pages 1.0

FooterButtonsPage {
    id: root

    property alias buttonGroup: buttonGroup
    property alias fullSyncButton: fullSyncButtonItem

    footerButtons.rightPrimary.enabled: false

    ColumnLayout {
        id: column

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 32

        HeaderTexts {
            id: header

            title: SyncsStrings.syncTitle
        }

        Item {
            id: spacer

            Layout.preferredHeight: 208
            Layout.preferredWidth: parent.width
            Layout.alignment: Qt.AlignLeft

            ButtonGroup {
                id: buttonGroup
            }

            RowLayout {
                id: row

                spacing: 15
                anchors.fill: parent

                SyncTypeButton {
                    id: fullSyncButtonItem

                    Layout.leftMargin: -fullSyncButtonItem.focusBorderWidth
                    title: SyncsStrings.fullSync
                    type: Constants.SyncType.FULL_SYNC
                    description: SyncsStrings.fullSyncButtonDescription
                    imageSource: Images.fullSync
                    imageSourceSize: Qt.size(172, 100)
                    ButtonGroup.group: buttonGroup
                    textHorizontalExtraMargin: 4
                    useMaxSiblingHeight: true
                }

                SyncTypeButton {
                    id: selectiveSyncButton

                    Layout.rightMargin: -selectiveSyncButton.focusBorderWidth
                    title: SyncsStrings.selectiveSync
                    type: Constants.SyncType.SELECTIVE_SYNC
                    description: SyncsStrings.selectiveSyncButtonDescription
                    imageSource: Images.selectiveSync
                    imageSourceSize: Qt.size(172, 100)
                    ButtonGroup.group: buttonGroup
                    textHorizontalExtraMargin: 4
                    useMaxSiblingHeight: true
                }
            }
        }

    } // ColumnLayout: column
}
