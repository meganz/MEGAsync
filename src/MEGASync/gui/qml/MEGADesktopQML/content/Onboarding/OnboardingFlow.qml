

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12

Item {
    //anchors.fill: parent
    id: rootItem

    StackView
    {
        anchors.fill:parent
        id: stack
        initialItem: welcomePage
    }
    Component{
        id: welcomePage
        Welcome{
            continueButton.onClicked: { stack.replace(loginPage)/*rootItem.state= "loginPage"*/}
        }
    }
    Component{
        id: loginPage
        RegisterFlow{}
//        LoginPage{
//            //returnButton.onClicked: {stack.replace(welcomePage) /*rootItem.state= "welcomePage"*/}
//        }
    }
}
