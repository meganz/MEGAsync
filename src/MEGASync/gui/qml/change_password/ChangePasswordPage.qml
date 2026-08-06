import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.pages 1.0
import components.textFields 1.0
import components.checkBoxes 1.0

import onboard 1.0
import PasswordStrengthChecker 1.0

FooterButtonsPage {
    id: root

    readonly property int minimumAllowedPasswordLength: 8
    readonly property int mainColumnDesignSpacing: 24

    readonly property int actionsGapDesign: 48
    readonly property int footerButtonsDesignHeight: 36 + 2 * Constants.focusBorderWidth
    readonly property real signOutRowHeight: Math.max(signOutCheckBox.height,
                                                      signOutTexts.y
                                                          + signOutLabel.height
                                                          + signOutTexts.spacing
                                                          + signOutHelper.height)
    readonly property real windowHeightHint: titleItem.height
                                             + passwordItem.height
                                             + (strengthStrip.visible
                                                ? passwordGroup.spacing + strengthStrip.height
                                                : 0)
                                             + confirmPasswordItem.height
                                             + signOutRowHeight
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
                    changePasswordComponentAccess.changePassword(passwordItem.text,
                                                                 signOutCheckBox.checked);
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

        Row {
            id: signOutRow

            width: parent.width
            spacing: 8

            CheckBox {
                id: signOutCheckBox

                // The indicator is an anchored Rectangle with no implicitWidth,
                // so a text-less CheckBox reports a near-zero width and the
                // indicator paints over the sibling texts. Size it explicitly.
                width: sizes.indicatorWidth + 2 * sizes.focusBorderWidth

                checked: true
            }

            Column {
                id: signOutTexts

                // The checkbox draws its 16px box inside a 4px focus border,
                // so the visual box sits focusBorderWidth below the row top;
                // offset the texts the same amount to align the label with it.
                y: Constants.focusBorderWidth

                width: parent.width - signOutCheckBox.width - signOutRow.spacing
                spacing: 8

                Texts.Text {
                    id: signOutLabel

                    width: parent.width
                    text: ChangePasswordStrings.signOutOtherDevices
                    font.weight: Font.DemiBold

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            signOutCheckBox.toggleCheckboxState();
                        }
                    }
                }

                Texts.Text {
                    id: signOutHelper

                    width: parent.width
                    color: ColorTheme.textSecondary
                    text: signOutCheckBox.checked
                          ? ChangePasswordStrings.signOutCheckedHelper
                          : ChangePasswordStrings.signOutUncheckedHelper
                }
            }
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
