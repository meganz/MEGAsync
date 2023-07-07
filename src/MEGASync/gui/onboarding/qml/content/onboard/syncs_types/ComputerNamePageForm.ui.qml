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

    footerButtons.previousButton.visible: false

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: contentMargin
        }
        spacing: 12

        Header {
            title: OnboardingStrings.computerNameTitle
            description: OnboardingStrings.computerNameDescription
        }

        MegaImages.SvgImage {
            source: Images.pcMega
            sourceSize: Qt.size(48, 48)
            color: Styles.textPrimary
        }

        MegaTextFields.TextField {
            id: computerNameTextField

            Layout.preferredWidth: parent.width
            title: OnboardingStrings.computerName
            textField.text: computerName.deviceName
            textField.enabled: false
        }
    }
}
