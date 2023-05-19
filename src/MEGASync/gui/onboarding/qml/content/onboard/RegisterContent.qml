// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Components 1.0 as Custom
import Common 1.0

//Local
import Onboard 1.0

Column {
    id: formColumn

    function error() {
        var error = firstName.text.length === 0 || lastName.text.length === 0;
        firstLastNameHint.visible = error;
        firstName.showType = error;
        lastName.showType = error;

        var valid = email.valid();
        if(!valid) {
            error = true;
            email.hintText = OnboardingStrings.errorValidEmail;
        }
        email.showType = !valid;
        email.hintVisible = !valid;

        valid = password.text.length !== 0;
        if(!valid) {
            error = true;
            password.hintType = Custom.HintText.Type.Error;
            password.type = Custom.TextField.Type.Error;
            password.showType = true;
        }
        password.showType = !valid;

        if(confirmPassword.text.length === 0) {
            error = true;
            confirmPassword.showType = true;
            confirmPassword.hintVisible = true;
            confirmPassword.hintText = OnboardingStrings.errorConfirmPassword;
            confirmPassword.type = Custom.TextField.Type.Error;
            confirmPassword.hintType = Custom.HintText.Type.Error;
        } else if(password.text !== confirmPassword.text) {
            error = true;
            confirmPassword.showType = true;
            confirmPassword.hintVisible = true;
            confirmPassword.type = Custom.TextField.Type.Error;
            confirmPassword.hintText = OnboardingStrings.errorPasswordsMatch;
            confirmPassword.hintType = Custom.HintText.Type.Error;
            password.hintVisible = false;
            password.type = Custom.TextField.Type.Error;
            password.showType = true;
        } else {
            confirmPassword.showType = false;
            confirmPassword.hintVisible = false;
        }

        return error;
    }

    function clean() {
        password.text = "";
        confirmPassword.text = "";
        firstName.text = "";
        lastName.text = "";
        email.text = "";
        termsCheckBox.checked = false;
        dataLossCheckBox.checked = false;
    }

    function showEmailAlreadyExistsError() {
        email.showType = true;
        email.hintText = OnboardingStrings.errorEmailAlreadyExist;
        email.hintVisible = true;
    }

    property alias firstName: firstName
    property alias lastName: lastName
    property alias firstLastNameHint: firstLastNameHint
    property alias email: email
    property alias password: password
    property alias confirmPassword: confirmPassword
    property alias termsCheckBox: termsCheckBox
    property alias dataLossCheckBox: dataLossCheckBox

    readonly property int contentWidth: 400
    readonly property int contentMargin: 48
    readonly property int checkboxSpacing: 16
    readonly property int checkboxBottomHeight: 1

    width: contentWidth
    spacing: contentMargin / 2

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 12

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 4

            Row {
                id: nameLayout

                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 8

                Custom.TextField {
                    id: firstName

                    width: nameLayout.width / 2 - nameLayout.spacing / 2
                    title: OnboardingStrings.firstName
                    type: Custom.TextField.Type.Error
                }

                Custom.TextField {
                    id: lastName

                    width: nameLayout.width / 2 - nameLayout.spacing / 2
                    title: OnboardingStrings.lastName
                    type: Custom.TextField.Type.Error
                }
            }

            Custom.HintText {
                id: firstLastNameHint

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: email.textField.focusBorderWidth
                anchors.rightMargin: email.textField.focusBorderWidth
                type: Custom.HintText.Type.Error
                text: OnboardingStrings.errorFirstLastName
            }
        }

        Custom.EmailTextField {
            id: email

            anchors.left: parent.left
            anchors.right: parent.right
            title: OnboardingStrings.email
        }

        Custom.PasswordTextField {
            id: password

            anchors.left: parent.left
            anchors.right: parent.right
            showHint: true
            title: OnboardingStrings.password
        }

        Custom.PasswordTextField {
            id: confirmPassword

            anchors.left: parent.left
            anchors.right: parent.right
            title: OnboardingStrings.confirmPassword
        }
    }

    Column {
        id: checksLayout

        anchors.left: parent.left
        anchors.right: parent.right
        spacing: checkboxSpacing

        Custom.CheckBox {
            id: dataLossCheckBox

            anchors.left: parent.left
            anchors.right: parent.right
            url: Links.security
            text: OnboardingStrings.understandLossPassword
        }

        Custom.CheckBox {
            id: termsCheckBox

            anchors.left: parent.left
            anchors.right: parent.right
            url: Links.terms
            text: OnboardingStrings.agreeTerms
        }

        Rectangle {
            height: checkboxBottomHeight
            anchors.left: parent.left
            anchors.right: parent.right
            color: "transparent"
        }
    }
}
