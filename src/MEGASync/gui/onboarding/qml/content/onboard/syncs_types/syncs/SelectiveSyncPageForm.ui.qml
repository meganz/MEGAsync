// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

// QML common
import Common 1.0

// Local
import Onboard.Syncs_types 1.0
import Onboard 1.0

SyncsPage {

    property alias localFolderChooser: localFolderChooser
    property alias remoteFolderChooser: remoteFolderChooser

    footerButtons.nextButton {
        enabled: localFolderChooser.isValid && remoteFolderChooser.isValid
        text: OnboardingStrings.sync
        iconSource: Images.sync
    }

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 32
        }
        spacing: 32

        Header {
            title: OnboardingStrings.selectiveSyncTitle
            description: OnboardingStrings.selectiveSyncDescription
        }

        InfoAccount {}

        ChooseSyncFolder {
            id: localFolderChooser
        }

        ChooseSyncFolder {
            id: remoteFolderChooser
            local: false
        }
    }

}
