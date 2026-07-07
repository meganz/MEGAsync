import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.pages 1.0

import SyncInfo 1.0

Item {
    id: root

    readonly property int imageSpacing: 0
    readonly property int textBottomSpacing: 60
    readonly property int designLineHeight: 20

    property alias image : imageItem
    property alias footerButtons: footerButtonsItem

    implicitHeight: mainLayout.height + Constants.defaultComponentSpacing

    ColumnLayout {
        id: mainLayout

        spacing: 0

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        Image {
            id: imageItem

            source: Images.megaDevices
        }

        Item {
            Layout.preferredHeight: root.imageSpacing
            Layout.preferredWidth: parent.width
        }

        Texts.Text {
            id: titleItem

            Layout.preferredWidth: parent.width
            text: SyncsStrings.finalStepSyncTitle
            font {
                pixelSize: Texts.Text.Size.LARGE
                weight: Font.Bold
            }
            wrapMode: Text.Wrap
        }

        Item {
            Layout.preferredHeight: Constants.defaultComponentSpacing
            Layout.preferredWidth: parent.width
        }

        Texts.SecondaryText {
            id: descriptionItem

            Layout.preferredWidth: parent.width
            text: SyncsStrings.finalStepSync
            font.pixelSize: Texts.Text.Size.MEDIUM
            wrapMode: Text.Wrap
            lineHeight: designLineHeight
            lineHeightMode: Text.FixedHeight
        }

        Item {
            Layout.preferredHeight: root.textBottomSpacing
            Layout.preferredWidth: parent.width
        }

        Item { // trick: wrapper to avoid the anchoring colision (inside the footerbuttons) with the layout manager. that's the only purpose.
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: footerButtonsItem.implicitHeight

            FooterButtons {
                id: footerButtonsItem

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
        }

        Item {
            Layout.preferredHeight: Constants.defaultComponentSpacing
            Layout.preferredWidth: parent.width
        }
    }
}
