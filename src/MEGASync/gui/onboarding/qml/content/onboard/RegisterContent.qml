// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Components.CheckBoxes 1.0 as MegaCheckBoxes
import Components.TextFields 1.0 as MegaTextFields
import Components.Texts 1.0 as MegaTexts
import Common 1.0

//Local
import Onboard 1.0

Column {
    id: formColumn

    function error() {
        var error = firstName.text.length === 0 || lastName.text.length === 0;
        firstLastNameHint.visible = error;
        firstName.error = error;
        lastName.error = error;

        var valid = email.valid();
        if(!valid) {
            error = true;
            email.hint.text = OnboardingStrings.errorValidEmail;
        }
        email.error = !valid;
        email.hint.visible = !valid;

        valid = password.text.length !== 0;
        if(!valid) {
            error = true;
            password.hint.type = MegaTexts.HintText.Type.Error;
            password.error = true;
        }
        password.error = !valid;

        if(confirmPassword.text.length === 0) {
            error = true;
            confirmPassword.error = true;
            confirmPassword.hint.visible = true;
            confirmPassword.hint.text = OnboardingStrings.errorConfirmPassword;
        } else if(password.text !== confirmPassword.text) {
            error = true;
            confirmPassword.error = true;
            confirmPassword.hint.visible = true;
            confirmPassword.type = MegaTextFields.TextField.Type.Error;
            confirmPassword.hint.text = OnboardingStrings.errorPasswordsMatch;
            password.hint.visible = false;
            password.error = true;
        } else {
            confirmPassword.error = false;
            confirmPassword.hint.visible = false;
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
        email.error = true;
        email.hint.text = OnboardingStrings.errorEmailAlreadyExist;
        email.hint.visible = true;
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

                MegaTextFields.TextField {
                    id: firstName

                    width: nameLayout.width / 2 - nameLayout.spacing / 2
                    title: OnboardingStrings.firstName
                }

                MegaTextFields.TextField {
                    id: lastName

                    width: nameLayout.width / 2 - nameLayout.spacing / 2
                    title: OnboardingStrings.lastName
                }
            }

            MegaTexts.HintText {
                id: firstLastNameHint

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: email.textField.focusBorderWidth
                anchors.rightMargin: email.textField.focusBorderWidth
                text: OnboardingStrings.errorFirstLastName
                icon: Images.alertTriangle
            }
        }

        MegaTextFields.EmailTextField {
            id: email

            anchors.left: parent.left
            anchors.right: parent.right
            title: OnboardingStrings.email
        }

        MegaTextFields.PasswordTextField {
            id: password

            anchors.left: parent.left
            anchors.right: parent.right
            showHint: true
            title: OnboardingStrings.password
        }

        MegaTextFields.PasswordTextField {
            id: confirmPassword

            anchors.left: parent.left
            anchors.right: parent.right
            title: OnboardingStrings.confirmPassword
            hint.icon: Images.alertTriangle
        }
    }

    Column {
        id: checksLayout

        anchors.left: parent.left
        anchors.right: parent.right
        spacing: checkboxSpacing

        MegaCheckBoxes.CheckBox {
            id: dataLossCheckBox

            anchors.left: parent.left
            anchors.right: parent.right
            url: Links.security
            text: OnboardingStrings.understandLossPassword
        }

        MegaCheckBoxes.CheckBox {
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
