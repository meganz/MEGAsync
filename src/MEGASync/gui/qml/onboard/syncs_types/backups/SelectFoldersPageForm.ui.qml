import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import onboard 1.0
import onboard.syncs_types 1.0

import BackupsModel 1.0

SyncsPage {

    footerButtons.rightPrimary {
        text: OnboardingStrings.backUp
        icons.source: Images.database
        enabled: backupsModelAccess.checkAllState !== Qt.Unchecked
    }

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 34

        Header {
            title: OnboardingStrings.selectBackupFoldersTitle
            description: OnboardingStrings.selectBackupFoldersDescription
        }

        ColumnLayout {

            Layout.preferredWidth: parent.width
            spacing: 12

            InfoAccount {
                Layout.preferredWidth: parent.width
            }

            SelectTable {}
        }
    }
}
