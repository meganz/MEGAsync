import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.accountData 1.0
import components.pages 1.0

import SyncInfo 1.0

import onboard 1.0

import syncs 1.0 as Syncs

FooterButtonsPage {
    id: root

    property alias confirmTable : contentItem
    readonly property int spacingConfirmSyncsTable: 7

    footerButtons {
        rightPrimary {
            text: Syncs.SyncsStrings.sync
            icons.source: Images.syncIcon
        }
    }

    ColumnLayout {
        id: column

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: Constants.defaultComponentSpacing

        HeaderTexts {
            id: header

            Layout.preferredWidth: parent.width
            title: OnboardingStrings.confirmSyncsWindowTitle
            description: OnboardingStrings.confirmSyncsWindowDescription
        }

        Column {
            id: syncData

            spacing: spacingConfirmSyncsTable

            InfoAccount {
                id: accountData
            }

            ConfirmSyncsTable {
                id: contentItem
            }
        }
    }

}
