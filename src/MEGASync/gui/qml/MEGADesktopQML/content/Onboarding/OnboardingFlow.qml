

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
    anchors.fill: parent
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
        LoginPage{
            //returnButton.onClicked: {stack.replace(welcomePage) /*rootItem.state= "welcomePage"*/}
        }
    }
}
/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.33;height:5000;width:5000}D{i:4;flowX:366.969696969697;flowY:230.15151515151518}
D{i:7;flowX:754.5454545454546;flowY:31.99999999999997}D{i:5;flowX:1270.9090909090908;flowY:263}
}
##^##*/

