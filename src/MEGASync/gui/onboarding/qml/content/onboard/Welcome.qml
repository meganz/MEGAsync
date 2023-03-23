import QtQuick 2.12
import QtQuick.Controls 2.12

WelcomeForm {

    continueButton.onClicked: {
        StackView.view.replace(loginPage);
    }
}
