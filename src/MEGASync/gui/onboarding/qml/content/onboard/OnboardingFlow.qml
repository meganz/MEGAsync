import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12

Item {
    id: rootItem

    StackView {
        id: stack

        anchors.fill:parent
        initialItem: welcomePage
    }

    Component {
        id: welcomePage

        Welcome {
            continueButton.onClicked: {
                stack.replace(loginPage);
            }
        }
    }

    Component{
        id: loginPage

        RegisterFlow {}
    }
}
