// System
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

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
            loginControllerAccess.createAccountErrorMsg = OnboardingStrings.errorValidEmail;
        }
        else
        {
            loginControllerAccess.createAccountErrorMsg = "";
        }

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

        MegaTextFields.EmailTextField {
            id: email

            anchors.left: parent.left
            anchors.leftMargin: -email.sizes.focusBorderWidth
            width: contentWidth + email.sizes.focusBorderWidth
            title: OnboardingStrings.email
            hint.icon: Images.mail
            hint.text: loginControllerAccess.createAccountErrorMsg;
            error: loginControllerAccess.createAccountErrorMsg.length !== 0;
            hint.visible: error;
        }

        MegaTextFields.PasswordTextField {
            id: password

            property bool validPassword: password.textField.text.length >= 8
                                            && passwordInfoPopup.validPassword

            anchors.left: parent.left
            anchors.leftMargin: -password.sizes.focusBorderWidth
            width: email.width
            title: OnboardingStrings.password
            cleanWhenError: false

            textField.onFocusChanged: {
                if (textField.focus) {
                    passwordInfoPopup.open();
                    hint.visible = false;
                } else {
                    var hintVisible = true;
                    if(textField.text.length < 8) {
                        hint.text = OnboardingStrings.minimum8Chars;
                        hint.styles.textColor = Styles.textError;
                    } else {
                        if(!passwordInfoPopup.validPassword) {
                            hint.text = OnboardingStrings.passwordEasilyGuessedError;
                            hint.styles.textColor = Styles.textError;
                        } else if(!passwordInfoPopup.allChecked) {
                            hint.text = OnboardingStrings.passwordEasilyGuessed;
                            hint.styles.textColor = Styles.textWarning;
                        } else {
                            hintVisible = false;
                        }
                    }
                    hint.visible = hintVisible;
                    passwordInfoPopup.close();
                }
            }

            PasswordInfoPopUp {
                id: passwordInfoPopup

                x: -335
                y: -54
                password: password.textField.text
            }

        }

        MegaTextFields.PasswordTextField {
            id: confirmPassword

            anchors.left: parent.left
            anchors.leftMargin: -confirmPassword.sizes.focusBorderWidth
            width: email.width
            title: OnboardingStrings.confirmPassword
            hint.icon: Images.key
            cleanWhenError: false
        }
    }

    MegaCheckBoxes.CheckBox {
        id: termsCheckBox

        anchors.left: parent.left
        anchors.right: parent.right
        url: Links.terms
        text: OnboardingStrings.agreeTerms
    }

    Component.onDestruction: {
        loginControllerAccess.createAccountErrorMsg ="";
    }
}
