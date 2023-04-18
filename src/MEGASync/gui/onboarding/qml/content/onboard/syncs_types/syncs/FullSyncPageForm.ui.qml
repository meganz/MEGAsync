// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

// QML common
import Components 1.0 as Custom

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

SyncsPage {

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 32
        }
        spacing: 32

        Header {
            title: OnboardingStrings.fullSyncTitle
            description: OnboardingStrings.fullSyncDescription
        }

        InfoAccount {}

        ChooseSyncFolder {}
    }

    footerButtons.nextButton.text: OnboardingStrings.sync
    footerButtons.nextButton.iconSource: Images.sync
}
