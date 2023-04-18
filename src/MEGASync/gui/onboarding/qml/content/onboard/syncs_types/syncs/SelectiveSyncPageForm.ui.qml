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

        ChooseSyncFolder {}

        ChooseSyncFolder {
            local: false
        }
    }

    footerButtons.nextButton.text: OnboardingStrings.sync
    footerButtons.nextButton.iconSource: Images.sync
}
