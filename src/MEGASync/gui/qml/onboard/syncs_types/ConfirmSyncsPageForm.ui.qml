import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.accountData 1.0
import components.pages 1.0

import SyncInfo 1.0

import onboard 1.0

FooterButtonsPage {
    id: root

    footerButtons {
        rightPrimary {
            text: SyncsStrings.sync
            icons.source: Images.syncIcon
        }
    }

    ColumnLayout {
        id: column

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 0
        }
        spacing: Constants.defaultComponentSpacing

        HeaderTexts {
            id: header

            Layout.preferredWidth: parent.width
            title: OnboardingStrings.selectiveSyncTitle
            description: OnboardingStrings.selectiveSyncDescription
        }

        InfoAccount {
            id: accountData

            Layout.preferredWidth: parent.width
            Layout.topMargin: 8
        }
    }

}
