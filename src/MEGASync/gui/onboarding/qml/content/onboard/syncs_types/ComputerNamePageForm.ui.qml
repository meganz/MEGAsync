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

    property alias computerNameTextField: computerNameTextField

    footerButtons.rightSecondary.visible: false

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 12

        Header {
            Layout.leftMargin: computerNameTextField.sizes.focusBorderWidth
            Layout.preferredWidth: parent.width
            title: OnboardingStrings.deviceNameTitle
            description: OnboardingStrings.deviceNameDescription
        }

        MegaImages.SvgImage {
            Layout.topMargin: 20
            Layout.leftMargin: computerNameTextField.sizes.focusBorderWidth
            source: Images.pcMega
            sourceSize: Qt.size(48, 48)
            color: Styles.textPrimary
        }

        MegaTextFields.TextField {
            id: computerNameTextField

            Layout.preferredWidth: parent.width
            title: OnboardingStrings.deviceName
            textField.text: computerName.deviceName
            sizes: MegaTextFields.LargeSizes {}
        }
    }
}
