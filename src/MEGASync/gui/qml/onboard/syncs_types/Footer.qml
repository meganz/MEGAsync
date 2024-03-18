import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.buttons 1.0

import onboard 1.0

RowLayout {
    id: root

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

    OutlineButton {
        id: leftSecondaryComp

        text: OnboardingStrings.skip
        onClicked: {
            onboardingWindow.close();
        }
    }

    RowLayout {
        id: buttonsLayout

        Layout.alignment: Qt.AlignRight

        OutlineButton {
            id: rightSecondaryComp

            text: OnboardingStrings.previous
        }

        PrimaryButton {
            id: rightPrimaryComp

            text: OnboardingStrings.next
            icons.source: Images.arrowRight
        }
    }
}
