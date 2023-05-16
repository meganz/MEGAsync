// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Components 1.0 as Custom
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

    Custom.Text {
        id: notNowButton

        text: OnboardingStrings.notNow
        font.weight: Font.Light
        font.underline: true
        Layout.leftMargin: 32
        Layout.bottomMargin: 24

        MouseArea {
            anchors.fill: notNowButton
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                Onboarding.exitLoggedIn();
            }
        }
    }

    RowLayout {
        Layout.alignment: Qt.AlignRight
        Layout.rightMargin: 32
        Layout.bottomMargin: 24

        Custom.Button {
            id: cancelButton

            text: OnboardingStrings.cancel
            onClicked: {
                Onboarding.exitLoggedIn(); // TODO: Logout?? Pending
            }
        }

        Custom.OutlineButton {
            id: previousButton

            text: OnboardingStrings.previous
        }

        Custom.PrimaryButton {
            id: nextButton

            text: OnboardingStrings.next
            iconSource: Images.arrowRight
        }

    }
}
