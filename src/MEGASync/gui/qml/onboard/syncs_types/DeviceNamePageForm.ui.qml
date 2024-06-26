import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.textFields 1.0
import components.images 1.0

import onboard 1.0

SyncsPage {
    id: root

    property alias deviceNameTextField: deviceNameTextFieldComp

    footerButtons.rightSecondary.visible: false

    ColumnLayout {
        id: mainLayout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 12

        Header {
            id: headerItem

            Layout.preferredWidth: parent.width
            title: OnboardingStrings.deviceNameTitle
            description: OnboardingStrings.deviceNameDescription
        }

        SvgImage {
            id: image

            Layout.topMargin: 20
            source: Images.pcMega
            sourceSize: Qt.size(48, 48)
            color: colorStyle.textPrimary
        }

        TextField {
            id: deviceNameTextFieldComp

            Layout.leftMargin: -sizes.focusBorderWidth
            Layout.rightMargin: -sizes.focusBorderWidth
            Layout.preferredWidth: parent.width + 2 * sizes.focusBorderWidth
            title: OnboardingStrings.deviceName
            hint.icon: ""
            sizes: LargeSizes {}
            textField {
                text: deviceName.name
                maximumLength: 32 // It is non-technical length limit for device name => UX choice
            }
        }
    }
}
