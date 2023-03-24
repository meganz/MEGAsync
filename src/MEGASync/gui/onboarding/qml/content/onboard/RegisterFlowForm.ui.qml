import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Components 1.0 as Custom
import Common 1.0

Rectangle {
    id: root

    color: Styles.alternateBackgroundColor
    border.color: "#ffffff"

    Image {
        id: image

        fillMode: Image.Tile
        source: "../../../../images/Onboarding/login_folder.png"
        anchors.left: root.left
        anchors.verticalCenter: root.verticalCenter
    }

    StackView {
        id: registerStack

        initialItem: loginPage
        anchors {
            left: image.right
            top: root.top
            bottom: root.bottom
            right: root.right
        }
    }

    LoginPage {
        id: loginPage
    }

} // Rectangle -> root
