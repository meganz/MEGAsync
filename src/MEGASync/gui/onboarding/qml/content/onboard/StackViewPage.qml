// System
import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts

//Local
import Onboarding 1.0
import Onboard 1.0
import LoginController 1.0

Rectangle {
    id: root
    
    property alias statusText: statusText

    readonly property int contentSpacing: 24

    function setInitialFocusPosition() {
        onboardingWindow.requestPageFocus();
    }

    function getStatusText() {
        switch(loginControllerAccess.state)
        {
            case LoginController.FETCHING_NODES:
            case LoginController.FETCHING_NODES_2FA:
            {
                return OnboardingStrings.statusFetchNodes;
            }
            case LoginController.LOGGING_IN:
            {
                return OnboardingStrings.statusLogin;
            }
            case LoginController.LOGGING_IN_2FA_VALIDATING:
            {
                return OnboardingStrings.status2FA;
            }
            case LoginController.CREATING_ACCOUNT:
            {
                return OnboardingStrings.statusSignUp;
            }
        }
        return "";
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
        text: getStatusText();
    }

}
