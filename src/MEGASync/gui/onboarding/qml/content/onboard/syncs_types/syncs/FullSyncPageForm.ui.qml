// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Components 1.0 as Custom
import Common 1.0

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

SyncsPage {

    property alias localFolderChooser: localFolderChooser

    footerButtons.nextButton {
        enabled: localFolderChooser.isValid
        text: OnboardingStrings.sync
        iconSource: Images.sync
        busyIndicatorImage: Images.loader
        progressBar: true
    }

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: contentMargin
        }
        spacing: 32

        Header {
            Layout.preferredWidth: parent.width
            title: OnboardingStrings.fullSyncTitle
            description: OnboardingStrings.fullSyncDescription
        }

        InfoAccount {
            Layout.preferredWidth: parent.width
        }

        ChooseSyncFolder {
            id: localFolderChooser

            Layout.preferredWidth: parent.width
            Layout.leftMargin: -4
        }
    }

}
