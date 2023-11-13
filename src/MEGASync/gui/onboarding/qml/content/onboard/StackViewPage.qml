// System
import QtQuick 2.15

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts

//Local
import Onboard 1.0

// C++
import LoginController 1.0

Rectangle {
    id: root
    
    readonly property int contentSpacing: 24
    readonly property int buttonsBottomMargin: 28

    signal initialFocus

    function setInitialFocusPosition() {
        root.initialFocus();
    }

    color: Styles.surface1

    MegaTexts.SecondaryText {
        id: statusText

        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: root.bottom
        }
        font.pixelSize: MegaTexts.Text.Size.Small
        color: Styles.textSecondary
        text: {
            switch(loginControllerAccess.state) {
                case LoginController.FETCHING_NODES:
                case LoginController.FETCHING_NODES_2FA:
                    return OnboardingStrings.statusFetchNodes;
                case LoginController.LOGGING_IN:
                    return OnboardingStrings.statusLogin;
                case LoginController.LOGGING_IN_2FA_VALIDATING:
                    return OnboardingStrings.status2FA;
                case LoginController.CREATING_ACCOUNT:
                    return OnboardingStrings.statusSignUp;
                default:
                    return "";
            }
        }
    }

}
