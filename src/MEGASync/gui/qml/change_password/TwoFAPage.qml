import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.buttons 1.0
import components.textFields 1.0
import components.texts 1.0 as Texts
import components.images 1.0
import components.pages 1.0

import onboard 1.0

FooterButtonsPage {
    id: root

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

    readonly property int mainColumnDesignSpacing : 24;
    readonly property int titleDescriptionDesignSpacing : 12;

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
        spacing: mainColumnDesignSpacing

        Image {
            id: lock

            source : Images.crystalLock
            sourceSize: Qt.size(96, 96)
            Layout.alignment: Qt.AlignHCenter
        }

        HeaderTexts {
            id: headerItem

            spacing: titleDescriptionDesignSpacing
            title: OnboardingStrings.twoFATitle
            titleWrapMode: Text.Wrap
            description: OnboardingStrings.twoFASubtitle
            titleWeight: Font.Normal
            descriptionFontSize: Texts.Text.Size.NORMAL
        }

        TwoFA {
            id: twoFAItem

            Layout.fillWidth: true
            focus: true

            onAllDigitsFilled: {
                check2FA();
            }
        }

    }

    Connections{
        id: twoFAConn

        target: changePasswordComponentAccess

        function onTwoFAVerificationFailed() {
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            root.enabled = true;
            twoFAItem.hasError = true
        }
    }
}
