

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Components 1.0 as Custom
import Styles 1.0

Rectangle {
    id: root
    color: Styles.alternateBackgroundColor
    border.color: "#ffffff"
    Image {
        id: image
        fillMode: Image.Tile
        source: "../../../../images/onboarding/login_folder.png"
        anchors.left: root.left
        anchors.verticalCenter: root.verticalCenter
    }
    StackView {
        id: registerStack
        anchors {
            left: image.right
            top: root.top
            bottom: root.bottom
            right: root.right
        }
        initialItem: loginPage
    }
    //    Component {
    //        id: loginPageComponent
    LoginPage {
        id: loginPage
        //    }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

