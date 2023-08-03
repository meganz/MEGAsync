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
        var error = false;

        var valid = email.valid();
        if(!valid) {
            error = true;
            email.hint.text = OnboardingStrings.errorValidEmail;
        }
        email.error = !valid;
        email.hint.visible = !valid;

        valid = password.text.length >= 8;
        if(!valid) {
            error = true;
            password.error = true;
            password.hint.text = OnboardingStrings.minimum8Chars;
        }
        password.error = !valid;
        password.hint.visible = !valid;

        if(confirmPassword.text.length === 0) {
            error = true;
            confirmPassword.error = true;
            confirmPassword.hint.visible = true;
            confirmPassword.hint.text = OnboardingStrings.errorConfirmPassword;
        } else if(password.text !== confirmPassword.text) {
            error = true;
            confirmPassword.error = true;
            confirmPassword.hint.visible = true;
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
    }

    function showEmailAlreadyExistsError() {
        email.error = true;
        email.hint.text = OnboardingStrings.errorEmailAlreadyExist;
        email.hint.visible = true;
    }

    property alias firstName: firstName
    property alias lastName: lastName
    property alias email: email
    property alias password: password
    property alias confirmPassword: confirmPassword
    property alias termsCheckBox: termsCheckBox

    readonly property int contentWidth: 402
    readonly property int contentMargin: 48
    readonly property int checkboxSpacing: 16
    readonly property int checkboxBottomHeight: 1

    width: contentWidth
    spacing: contentMargin / 2

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 14

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 4

            Row {
                id: nameLayout

                anchors.left: parent.left
                anchors.leftMargin: -firstName.sizes.focusBorderWidth
                width: email.width
                spacing: 8

                MegaTextFields.TextField {
                    id: firstName

                    width: nameLayout.width / 2 - nameLayout.spacing / 2
                    title: OnboardingStrings.firstName
                    hint.icon: Images.person
                    hint.text: OnboardingStrings.errorName
                }

                MegaTextFields.TextField {
                    id: lastName

                    width: nameLayout.width / 2 - nameLayout.spacing / 2
                    title: OnboardingStrings.lastName
                    hint.icon: Images.person
                    hint.text: OnboardingStrings.errorLastName
                }
            }
        }

        MegaTextFields.EmailTextField {
            id: email

            anchors.left: parent.left
            anchors.leftMargin: -email.sizes.focusBorderWidth
            width: contentWidth + email.sizes.focusBorderWidth
            title: OnboardingStrings.email
            hint.icon: Images.mail
        }

        MegaTextFields.PasswordTextField {
            id: password

            anchors.left: parent.left
            anchors.leftMargin: -password.sizes.focusBorderWidth
            width: email.width
            title: OnboardingStrings.password

            textField.onTextChanged: {
                passwordInfoPopup.content.checkPasswordConditions(text);
            }

            textField.onFocusChanged: {
                if (textField.focus) {
                    passwordInfoPopup.open();
                    hint.visible = false;
                } else {
                    if(textField.text.length < 8) {
                        hint.text = OnboardingStrings.minimum8Chars;
                        hint.styles.textColor = Styles.textError;
                        hint.visible = true;
                    } else if(!passwordInfoPopup.content.allChecked()) {
                        hint.text = OnboardingStrings.passwordEasilyGuessed;
                        hint.styles.textColor = Styles.textWarning;
                        hint.visible = true;
                    }
                    passwordInfoPopup.close();
                }
            }

            PasswordInfoPopUp {
                id: passwordInfoPopup

                x: -335
                y: -78
            }
        }

        MegaTextFields.PasswordTextField {
            id: confirmPassword

            anchors.left: parent.left
            anchors.leftMargin: -confirmPassword.sizes.focusBorderWidth
            width: email.width
            title: OnboardingStrings.confirmPassword
            hint.icon: Images.key
        }
    }

    MegaCheckBoxes.CheckBox {
        id: termsCheckBox

        anchors.left: parent.left
        anchors.right: parent.right
        url: Links.terms
        text: OnboardingStrings.agreeTerms
    }
}
