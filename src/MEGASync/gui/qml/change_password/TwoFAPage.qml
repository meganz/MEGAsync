import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.buttons 1.0
import components.textFields 1.0
import components.texts 1.0 as Texts
import components.pages 1.0

import onboard 1.0

FooterButtonsPage {
    id: root

    readonly property int imageContentDesignSpacing: 10
    readonly property int mainColumnDesignSpacing: 24
    readonly property int titleDescriptionDesignSpacing: 8
    readonly property int keypadImageSize: 120

    footerButtons {
        leftPrimary.visible: false
        rightSecondary {
            visible: true
            text: Strings.cancel

            onClicked: {
                window.close();
            }
        }

        rightPrimary {
            text: Strings.next
            icons: Icon {}

            enabled: twoFAItem.hasAllDigitsFilled;

            onClicked: {
                check2FA();
            }
        }
    }

    function check2FA() {
        changePasswordComponentAccess.check2FA(twoFAItem.key);
        footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
        root.enabled = false;
    }

    ColumnLayout {
        id: mainColumn

        anchors {
            left: root.left
            right: root.right
            top: root.top
            topMargin: 0
        }
        spacing: imageContentDesignSpacing

        Image {
            id: keypadImage

            // No sourceSize here: it would force the decode to 120px and blur
            // on high-DPI displays. Qt picks the @2x/@3x variant instead.
            source: Images.twoFAKeypad
            Layout.preferredWidth: keypadImageSize
            Layout.preferredHeight: keypadImageSize
        }

        ColumnLayout {
            id: formColumn

            Layout.fillWidth: true
            spacing: mainColumnDesignSpacing

            ColumnLayout {
                id: headerColumn

                Layout.fillWidth: true
                spacing: titleDescriptionDesignSpacing

                Texts.Text {
                    id: titleItem

                    Layout.fillWidth: true
                    text: ChangePasswordStrings.twoFATitle
                    font {
                        pixelSize: Texts.Text.Size.LARGE_SMALL
                        weight: Font.Bold
                    }
                    lineHeight: 28
                    lineHeightMode: Text.FixedHeight
                }

                Texts.Text {
                    id: descriptionItem

                    Layout.fillWidth: true
                    text: OnboardingStrings.twoFASubtitle
                    color: ColorTheme.textSecondary
                    wrapMode: Text.WordWrap
                }
            }

            TwoFA {
                id: twoFAItem

                Layout.fillWidth: true
                focus: true
                inlineError: true
                inlineErrorText: ChangePasswordStrings.incorrect2FACode
                digitsSpacing: 4
                // Replaced by the design-accurate underlined link below; its
                // recovery-URL logic is still triggered through the alias.
                helpButton.visible: false

                onAllDigitsFilled: {
                    check2FA();
                }
            }

            Texts.Text {
                id: lostDeviceLink

                text: ChangePasswordStrings.lostAuthenticatorDevice
                color: ColorTheme.linkPrimary
                font {
                    pixelSize: Texts.Text.Size.MEDIUM
                    underline: true
                }
                lineHeight: 20
                lineHeightMode: Text.FixedHeight

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        // Same recovery-URL flow as the hidden TwoFA help link.
                        twoFAItem.helpButton.clicked();
                    }
                }
            }
        }
    }

    Connections {
        id: twoFAConn

        target: changePasswordComponentAccess

        function onTwoFAVerificationFailed() {
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            root.enabled = true;
            twoFAItem.hasError = true;
        }
    }
}
