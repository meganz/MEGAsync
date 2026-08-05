import QtQuick 2.15

import common 1.0

import components.views 1.0
import components.textFields 1.0

import ChangePasswordComponents 1.0

ChangePasswordDialog {
    id: window

    // Locked while the SDK request is in flight: Cancel disabled, Esc and the
    // close button ignored until the change resolves (SNC-2109). The close
    // guard lives in ChangePasswordDialog::event (QEvent::Close) because the
    // `closing` signal is revisioned and not exposed by this registered type.
    property bool busy: false

    // 2FA dialog geometry from the Figma frames (SNC-2109, node 3744:1486):
    // 611x482 card, plus ~4px of extra room for the inline error line.
    readonly property int twoFAPageHeigh: 486
    readonly property int twoFAPageWidth: 611
    readonly property int passwordChangePageHeigh: 420
    readonly property int passwordChangePageWidth: 448

    // The dialog height follows the page content (SNC-2109 auto-layout):
    // the page exposes windowHeightHint (content + 48px gap + footer) and the
    // window resizes to it, keeping the fixed-size min/max constraints.
    property real pageHeightHint: (stackView.currentItem
                                   && stackView.currentItem.windowHeightHint !== undefined)
                                  ? stackView.currentItem.windowHeightHint
                                  : 0

    onPageHeightHintChanged: {
        applyPageHeight();
    }

    // Apply the initial page height before the dialog is shown, so the first
    // visible frame already has the footer at its final position.
    Component.onCompleted: {
        applyPageHeight();
    }

    function applyPageHeight() {
        if (pageHeightHint <= 0
                || changePasswordContentItem.state === changePasswordContentItem.two_fa) {
            return;
        }

        var newHeight = Math.round(pageHeightHint);
        // Same clamping order trick as the two_fa state below: raise the
        // maximum before the height when growing, and lower the minimum
        // before the height when shrinking.
        if (newHeight > window.height) {
            window.maximumHeight = newHeight;
            window.height = newHeight;
            window.minimumHeight = newHeight;
        }
        else {
            window.minimumHeight = newHeight;
            window.height = newHeight;
            window.maximumHeight = newHeight;
        }
    }

    title: ChangePasswordStrings.title
    visible: false
    modality: Qt.WindowModal
    color: ColorTheme.pageBackground
    width: passwordChangePageWidth
    height: passwordChangePageHeigh
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width

    Rectangle {
        id: changePasswordContentItem

        anchors.fill: parent
        color: ColorTheme.pageBackground

        readonly property string change_password: "change_password"
        readonly property string two_fa: "two_fa"

        states: [
            State {
                name: changePasswordContentItem.change_password
                StateChangeScript {
                    script: stackView.replace(changePasswordPage);
                }
            },
            State {
                name: changePasswordContentItem.two_fa
                StateChangeScript {
                    script: {
                        // Grow the window before switching page. The maximums are
                        // raised before the sizes so the resize is not clamped by the
                        // stale change-password constraints, which would otherwise
                        // leave the window at the old size until a geometry event
                        // (e.g. moving the dialog) re-applied the constraints.
                        window.maximumHeight = twoFAPageHeigh;
                        window.maximumWidth = twoFAPageWidth;
                        window.height = twoFAPageHeigh;
                        window.width = twoFAPageWidth;
                        window.minimumHeight = twoFAPageHeigh;
                        window.minimumWidth = twoFAPageWidth;
                        stackView.replace(twoFAPage);
                    }
                }
            }
        ]

        StackViewBase {
            id: stackView

            anchors {
                fill: parent
                // 24px dialog padding (SNC-2109)
                margins: Constants.defaultComponentSpacing
            }

            initialItem: changePasswordPage

            Component {
                id: changePasswordPage

                ChangePasswordPage {
                    id: changePasswordItem
                }
            }

            Component {
                id: twoFAPage

                TwoFAPage {
                    id: twoFAItem
                }
            }
        }

        Connections{
            id: changePassConn

            target: changePasswordComponentAccess

            function onShow2FA() {
                window.busy = false;
                changePasswordContentItem.state = changePasswordContentItem.two_fa;
            }

            function onPasswordChangeSucceed() {
                // Release the lock before closing: ChangePasswordDialog::event
                // ignores QEvent::Close while `busy`, and QWindow::close()
                // dispatches through event() just like the title bar button,
                // so closing here without clearing it leaves the dialog open
                // and inert (SNC-2109).
                window.busy = false;
                window.close();
            }
        }
    }
}
