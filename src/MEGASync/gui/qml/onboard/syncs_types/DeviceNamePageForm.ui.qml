import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.textFields 1.0
import components.images 1.0

import onboard 1.0

SyncsPage {

    property alias deviceNameTextField: deviceNameTextFieldComp

    footerButtons.rightSecondary.visible: false

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 12

        Header {
            Layout.preferredWidth: parent.width
            title: OnboardingStrings.deviceNameTitle
            description: OnboardingStrings.deviceNameDescription
        }

        SvgImage {
            Layout.topMargin: 20
            source: Images.pcMega
            sourceSize: Qt.size(48, 48)
            color: Styles.textPrimary
        }

        TextField {
            id: deviceNameTextFieldComp

            Layout.leftMargin: -sizes.focusBorderWidth
            Layout.rightMargin: -sizes.focusBorderWidth
            Layout.preferredWidth: parent.width + 2 * sizes.focusBorderWidth
            title: OnboardingStrings.deviceName
            textField.text: deviceName.name
            textField.maximumLength: 32//32 is non-technical length limit for device name
                                       //UX choice
            hint.icon: ""
            sizes: LargeSizes {}
        }
    }
}
