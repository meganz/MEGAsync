// System
import QtQuick 2.15

// QML common
import common 1.0
import Components.CheckBoxes 1.0 as MegaCheckBoxes
import components.textFields 1.0
import components.texts 1.0

//Local
import onboard 1.0

Column {
    id: root

    readonly property int contentWidth: 403
    readonly property int contentMargin: 48
    readonly property int checkboxSpacing: 16
    readonly property int mainFormLayoutSpacing: 14
    readonly property int nameLayoutSpacing: 8
    readonly property int xPositionPasswordPopup: -335
    readonly property int yPositionPasswordPopup: -54

    property alias firstName: firstNameItem
    property alias lastName: lastNameItem
    property alias email: emailItem
    property alias password: passwordItem
    property alias confirmPassword: confirmPasswordItem
    property alias termsCheckBox: termsCheckBoxItem

    function error() {
        var error = false;

        var valid = emailItem.valid();
        if(!valid) {
            error = true;
            loginControllerAccess.createAccountErrorMsg = OnboardingStrings.errorValidEmail;
        }
        else {
            loginControllerAccess.createAccountErrorMsg = "";
        }

        valid = passwordItem.text.length >= 8;
        if(!valid) {
            error = true;
            passwordItem.error = true;
            passwordItem.hint.text = OnboardingStrings.minimum8Chars;
        }
        passwordItem.error = !valid;
        passwordItem.hint.visible = !valid;

        if(confirmPasswordItem.text.length === 0) {
            error = true;
            confirmPasswordItem.error = true;
            confirmPasswordItem.hint.visible = true;
            confirmPasswordItem.hint.text = OnboardingStrings.errorConfirmPassword;
        }
        else if(passwordItem.text !== confirmPasswordItem.text) {
            error = true;
            confirmPasswordItem.error = true;
            confirmPasswordItem.hint.visible = true;
            confirmPasswordItem.hint.text = OnboardingStrings.errorPasswordsMatch;
            passwordItem.hint.visible = false;
            passwordItem.error = true;
        }
        else {
            confirmPasswordItem.error = false;
            confirmPasswordItem.hint.visible = false;
        }

        return error;
    }

    function clean() {
        passwordItem.text = "";
        confirmPasswordItem.text = "";
        firstNameItem.text = "";
        lastNameItem.text = "";
        emailItem.text = "";
        termsCheckBoxItem.checked = false;
    }

    width: contentWidth
    spacing: contentMargin / 2 - termsCheckBoxItem.sizes.focusBorderWidth

    Component.onDestruction: {
        loginControllerAccess.createAccountErrorMsg = "";
    }

    Column {
        id: mainFormLayout

        anchors {
            left: root.left
            right: root.right
            leftMargin: -firstNameItem.sizes.focusBorderWidth
        }
        spacing: mainFormLayoutSpacing

        Row {
            id: nameLayout

            width: emailItem.width
            spacing: nameLayoutSpacing

            TextField {
                id: firstNameItem

                width: nameLayout.width / 2 - nameLayout.spacing / 2
                title: OnboardingStrings.firstName
                hint.icon: Images.person
                hint.text: OnboardingStrings.errorName
            }

            TextField {
                id: lastNameItem

                width: nameLayout.width / 2 - nameLayout.spacing / 2
                title: OnboardingStrings.lastName
                hint.icon: Images.person
                hint.text: OnboardingStrings.errorLastName
            }
        }

        MegaTextFields.EmailTextField {
            id: emailItem

            width: contentWidth + emailItem.sizes.focusBorderWidth
            title: OnboardingStrings.email
            hint.icon: Images.mail
            hint.text: loginControllerAccess.createAccountErrorMsg;
            error: loginControllerAccess.createAccountErrorMsg.length !== 0
            hint.visible: error;
        }

        MegaTextFields.PasswordTextField {
            id: passwordItem

            property bool validPassword: passwordItem.textField.text.length >= 8
                                            && passwordInfoPopup.validPassword

            width: emailItem.width
            title: OnboardingStrings.password
            cleanWhenError: false

            textField.onFocusChanged: {
                if (textField.focus) {
                    passwordInfoPopup.open();
                    hint.visible = false;
                }
                else {
                    var hintVisible = true;
                    if(textField.text.length < 8) {
                        hint.text = OnboardingStrings.minimum8Chars;
                        hint.textColor = Styles.textError;
                    }
                    else {
                        if(!passwordInfoPopup.validPassword) {
                            hint.text = OnboardingStrings.passwordEasilyGuessedError;
                            hint.textColor = Styles.textError;
                        }
                        else if(!passwordInfoPopup.allChecked) {
                            hint.text = OnboardingStrings.passwordEasilyGuessed;
                            hint.textColor = Styles.textWarning;
                        }
                        else {
                            hintVisible = false;
                        }
                    }
                    hint.visible = hintVisible;
                    passwordInfoPopup.close();
                }
            }

            PasswordInfoPopUp {
                id: passwordInfoPopup

                x: xPositionPasswordPopup
                y: yPositionPasswordPopup
                password: passwordItem.textField.text
            }

        }

        MegaTextFields.PasswordTextField {
            id: confirmPasswordItem

            width: emailItem.width
            title: OnboardingStrings.confirmPassword
            hint.icon: Images.key
            cleanWhenError: false
        }
    }

    MegaCheckBoxes.CheckBox {
        id: termsCheckBoxItem

        anchors.left: root.left
        anchors.leftMargin: -termsCheckBoxItem.sizes.focusBorderWidth
        url: Links.terms
        text: OnboardingStrings.agreeTerms
    }

}
