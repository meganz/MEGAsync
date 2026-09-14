import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.pages 1.0
import components.textFields 1.0

import onboard 1.0
import PasswordStrengthChecker 1.0

FooterButtonsPage {
    id: root

    readonly property int minimumAllowedPasswordLength: 8
    readonly property int mainColumnDesignSpacing: 24

    readonly property int actionsGapDesign: 48
    readonly property int footerButtonsDesignHeight: 36 + 2 * Constants.focusBorderWidth
    readonly property real windowHeightHint: titleItem.height
                                             + passwordItem.height
                                             + (strengthStrip.visible
                                                ? passwordGroup.spacing + strengthStrip.height
                                                : 0)
                                             + confirmPasswordItem.height
                                             + otherSessionsInfo.height
                                             + 3 * mainColumnDesignSpacing
                                             + actionsGapDesign
                                             + footerButtonsDesignHeight
                                             + 2 * Constants.defaultComponentSpacing
                                             + Constants.focusAdjustment
                                             - Constants.focusBorderWidth

    Component.onCompleted: {
        Qt.callLater(() => passwordItem.textField.forceActiveFocus())
    }

    Keys.onPressed: (event)=>{
        if (event.key === Qt.Key_Escape) {
            event.accepted = true;
            if (!window.busy) {
                window.close();
            }
        }
    }

    function setError(errorMessage) {
        window.busy = false;
        passwordItem.submitError = errorMessage;
    }

    function validate() {
        if (passwordItem.text.length < minimumAllowedPasswordLength) {
            passwordItem.submitError = ChangePasswordStrings.useAtLeast8Characters;
            return false;
        }
        if (passwordItem.text !== confirmPasswordItem.text) {
            confirmPasswordItem.hint.text = OnboardingStrings.errorPasswordsMatch;
            confirmPasswordItem.hint.visible = true;
            confirmPasswordItem.error = true;
            return false;
        }
        return true;
    }

    footerButtons {
        leftPrimary.visible: false

        rightSecondary {
            visible: true
            text: Strings.cancel
            enabled: !window.busy

            onClicked: {
                window.close();
            }
        }

        rightPrimary {
            text: window.busy ? "" : ChangePasswordStrings.title
            icons: Icon {
                busyIndicatorVisible: window.busy
            }
            enabled: window.busy
                     || (passwordItem.text !== "" && confirmPasswordItem.text !== "")

            onClicked: {
                if (!window.busy && validate()) {
                    window.busy = true;
                    changePasswordComponentAccess.changePassword(passwordItem.text);
                }
            }
        }
    }

    Column {
        id: mainFormLayout

        anchors.fill: parent
        spacing: mainColumnDesignSpacing
        enabled: !window.busy

        Texts.Text {
            id: titleItem

            width: parent.width
            text: ChangePasswordStrings.title
            font {
                pixelSize: Texts.Text.Size.LARGE
                weight: Font.DemiBold
            }
        }

        Column {
            id: passwordGroup

            width: parent.width
            spacing: 4

            PasswordTextField {
                id: passwordItem

                property string submitError: ""

                width: parent.width
                title: ChangePasswordStrings.newPassword
                placeholderText: ChangePasswordStrings.enterNewPassword
                clearEnabled: true
                cleanWhenError: false
                error: submitError !== ""

                textField.onTextChanged: {
                    submitError = "";
                }

                PasswordStrengthChecker {
                    id: passChecker

                    password: passwordItem.text
                }
            }

            Item {
                id: strengthStrip

                // Strip for the meter + hint (or the submit error). Its height
                // is state-driven so the dialog grows/shrinks with the content
                // (Figma auto-layout), but it is computed from design constants
                // rather than from the children: positioner heights only
                // settle on polish and hidden HintTexts report zero height,
                // which would feed windowHeightHint too late (post-show).
                readonly property int meterDesignHeight: 10
                readonly property int hintLineDesignHeight: 18

                width: parent.width
                height: {
                    if (passwordItem.submitError !== "") {
                        // The error replaces the meter: hint line only.
                        return hintLineDesignHeight;
                    }
                    if (passwordItem.text !== "") {
                        return meterDesignHeight + strip.spacing + hintLineDesignHeight;
                    }
                    return 0;
                }
                visible: height > 0

                Column {
                    id: strip

                    anchors.fill: parent
                    spacing: 2

                    PasswordStrengthIndicator {
                        id: strengthMeter

                        width: parent.width
                        visible: passwordItem.text !== "" && passwordItem.submitError === ""
                        strength: passChecker.strength
                    }

                    Texts.HintText {
                        id: strengthHint

                        readonly property bool isSubmitError: passwordItem.submitError !== ""

                        width: parent.width
                        visible: isSubmitError || passwordItem.text !== ""
                        icon: (isSubmitError || strengthMeter.isWeak)
                              ? Images.alertTriangle
                              : (strengthMeter.isModerate ? Images.alertCircle
                                                          : Images.checkCircle)
                        iconColor: strengthHint.textColor
                        textColor: (isSubmitError || strengthMeter.isWeak)
                                   ? ColorTheme.textError
                                   : (strengthMeter.isModerate ? ColorTheme.textWarning
                                                               : ColorTheme.textSuccess)
                        text: isSubmitError
                              ? passwordItem.submitError
                              : (strengthMeter.isWeak
                                 ? ChangePasswordStrings.weakPassword
                                 : (strengthMeter.isModerate
                                    ? ChangePasswordStrings.moderatePassword
                                    : ChangePasswordStrings.strongPassword))
                    }
                }
            }
        }

        PasswordTextField {
            id: confirmPasswordItem

            width: parent.width
            title: ChangePasswordStrings.confirmNewPassword
            placeholderText: ChangePasswordStrings.reEnterPassword
            clearEnabled: true
            cleanWhenError: false
            hint.icon: Images.alertTriangle

            textField.onTextChanged: {
                error = false;
                hint.visible = false;
            }
        }

        // The API signs out every other session as part of the password
        // change; there is no opt-out yet (API-4286), so the former
        // "Sign out of all other devices" checkbox is replaced by this
        // informational line. SNC-6910
        Texts.Text {
            id: otherSessionsInfo

            // The text fields draw their title and box inside a
            // focusBorderWidth inset; use the same inset so this line is
            // flush with the field titles above it.
            x: Constants.focusBorderWidth
            width: parent.width - 2 * Constants.focusBorderWidth
            // Follow the enabled state like the rest of the form: while the
            // request is in flight the page is disabled and everything else
            // dims, so a fixed colour here would make this line stand out.
            color: enabled ? ColorTheme.textSecondary : ColorTheme.textDisabled
            text: ChangePasswordStrings.otherSessionsInfo
        }
    }

    Connections {
        id: changePassConn

        target: changePasswordComponentAccess

        function onPasswordChangeFailed(errorMessage) {
            setError(errorMessage);
        }

        function onPasswordCheckFailed(errorMessage) {
            setError(errorMessage);
        }
    }
}
