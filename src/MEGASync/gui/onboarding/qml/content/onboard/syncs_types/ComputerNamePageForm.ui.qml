// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Components 1.0 as Custom
import Common 1.0

// Local
import Onboard 1.0

SyncsPage {

    property alias computerNameTextField: computerNameTextField

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

        Custom.SvgImage {
            source: Images.pcMega
            sourceSize: Qt.size(48, 48)
            color: Styles.textPrimary
        }

        Custom.TextField {
            id: computerNameTextField

            Layout.preferredWidth: parent.width
            Layout.leftMargin: -focusWidth
            title: OnboardingStrings.computerName
        }
    }
}
