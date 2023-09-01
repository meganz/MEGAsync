// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Components.TextFields 1.0 as MegaTextFields
import Components.Images 1.0 as MegaImages
import Common 1.0

// Local
import Onboard 1.0

SyncsPage {

    property alias deviceNameTextField: deviceNameTextField

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

        MegaImages.SvgImage {
            Layout.topMargin: 20
            source: Images.pcMega
            sourceSize: Qt.size(48, 48)
            color: Styles.textPrimary
        }

        MegaTextFields.TextField {
            id: deviceNameTextField

            Layout.leftMargin: -deviceNameTextField.sizes.focusBorderWidth
            Layout.rightMargin: -deviceNameTextField.sizes.focusBorderWidth
            Layout.preferredWidth: parent.width + 2 * deviceNameTextField.sizes.focusBorderWidth
            title: OnboardingStrings.deviceName
            textField.text: deviceName.name
            sizes: MegaTextFields.LargeSizes {}
            hint.icon: Images.alertTriangle
        }
    }
}
