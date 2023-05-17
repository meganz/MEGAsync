// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0

// Local
import Onboard.Syncs_types 1.0
import Onboard 1.0

SyncsPage {

    property alias localFolderChooser: localFolderChooser
    property alias remoteFolderChooser: remoteFolderChooser
    color: "#ffffff"
    border.color: "#ffffff"

    footerButtons.nextButton {
        enabled: localFolderChooser.isValid && remoteFolderChooser.isValid
        text: OnboardingStrings.sync
        icons.source: Images.sync
        icons.busyIndicatorImage: Images.loader
    }

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: contentMargin
        }
        spacing: contentMargin

        Header {
            Layout.preferredWidth: parent.width
            title: OnboardingStrings.selectiveSyncTitle
            description: OnboardingStrings.selectiveSyncDescription
        }

        InfoAccount {
            Layout.preferredWidth: parent.width
        }

        ChooseSyncFolder {
            id: localFolderChooser

            Layout.preferredWidth: parent.width
            Layout.leftMargin: -4
        }

        ChooseSyncFolder {
            id: remoteFolderChooser

            Layout.preferredWidth: parent.width
            Layout.leftMargin: -4
            local: false
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

