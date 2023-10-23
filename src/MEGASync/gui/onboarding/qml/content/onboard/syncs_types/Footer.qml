// System
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

// QML common
import Components.Buttons 1.0 as MegaButtons
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

RowLayout {

    property alias leftSecondary: leftSecondaryComp
    property alias rightSecondary: rightSecondaryComp
    property alias rightPrimary: rightPrimaryComp

    anchors {
        bottom: parent.bottom
        right: parent.right
        left: parent.left
        leftMargin: -leftSecondary.sizes.focusBorderWidth
        bottomMargin: -leftSecondary.sizes.focusBorderWidth
        rightMargin: -rightPrimary.sizes.focusBorderWidth
    }

    MegaButtons.OutlineButton {
        id: leftSecondaryComp

        text: OnboardingStrings.skip
        onClicked: {
            onboardingWindow.close();
        }
    }

    RowLayout {
        Layout.alignment: Qt.AlignRight

        MegaButtons.OutlineButton {
            id: rightSecondaryComp

            text: OnboardingStrings.previous
        }

        MegaButtons.PrimaryButton {
            id: rightPrimaryComp

            text: OnboardingStrings.next
            icons.source: Images.arrowRight
        }

    }
}
