// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Components.Buttons 1.0 as MegaButtons
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

RowLayout {

    property alias notNowButton: notNowButton
    property alias previousButton: previousButton
    property alias nextButton: nextButton

    width: parent.width

    MegaButtons.SecondaryButton {
        id: notNowButton

        text: OnboardingStrings.notNow
        onClicked: {
            Onboarding.exitLoggedIn();
        }
    }

    RowLayout {
        Layout.alignment: Qt.AlignRight

        MegaButtons.OutlineButton {
            id: previousButton

            text: OnboardingStrings.previous
        }

        MegaButtons.PrimaryButton {
            id: nextButton

            text: OnboardingStrings.next
            icons.source: Images.arrowRight
        }

    }
}
