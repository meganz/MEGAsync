// System
import QtQuick 2.15
import QtQuick.Layouts 1.15

// QML common
import Common 1.0

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

// C++
import BackupsModel 1.0

SyncsPage {

    footerButtons.rightPrimary {
        text: OnboardingStrings.backUp
        icons.source: Images.database
        enabled: BackupsModel.mCheckAllState !== Qt.Unchecked
    }

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 24

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
