// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtGraphicalEffects 1.0

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Components.Buttons 1.0 as MegaButtons
import Components.ProgressBars 1.0 as MegaProgressBars

// Local
import Onboard 1.0
import Guest 1.0

// C++
import GuestContent 1.0
import ApiEnums 1.0
import LoginController 1.0

Rectangle {
    id: content

    property string title: ""
    property string description: ""
    property bool indeterminate: true
    property double progressValue: 0.0
    property bool validating2FA: false

    readonly property string stateNormal: "NORMAL"
    readonly property string stateInProgress: "IN_PROGRESS"

    width: 400
    height: 560
    radius: 10
    color: Styles.surface1

    state: content.stateNormal
    states: [
        State {
            name: content.stateNormal
            StateChangeScript {
                script: stack.replace(initialPage);
            }
        },
        State {
            name: content.stateInProgress
            StateChangeScript {
                script: stack.replace(progressPage);
            }
        }
    ]

    MegaImages.SvgImage {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 16
        anchors.leftMargin: 16
        source: Images.mega
        sourceSize: Qt.size(24, 24)
    }

    MegaButtons.IconButton {
        id: menuButton

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 9
        anchors.rightMargin: 9
        icons.source: Images.menu
        onClicked: {
            menuButton.checked = true;
            menu.visible = !menu.visible;
        }
    }

    Qml.Menu {
        id: menu

        x: menuButton.x + menuButton.width - width
        y: menuButton.y + menuButton.width + 4
        dim: true
        modal: true

        Qml.Overlay.modal: Rectangle {
            color: "transparent"
        }

        background: Rectangle {
            implicitWidth: 200
            implicitHeight: 120
            color: Styles.pageBackground
            radius: 12
            layer.enabled: true
            layer.effect: DropShadow {
                transparentBorder: true
                horizontalOffset: 4
                verticalOffset: 8
                radius: 8.0
                samples: 16
                cached: true
                color: "#10000000"
            }
        }

        onClosed: {
            menuButton.checked = false;
        }

        MenuItem {
            id: aboutMenuItem

            text: GuestStrings.menuAboutMEGA
            icon.source: Images.megaOutline
            position: MenuItem.Position.First
            onTriggered: {
                GuestContent.onAboutMEGAClicked();
                guestWindow.hide();
            }
        }

        MenuItem {
            id: preferencesItem

            text: GuestStrings.menuPreferences
            icon.source: Images.settings
            onTriggered: {
                GuestContent.onPreferencesClicked();
                guestWindow.hide();
            }
        }

        MenuItem {
            id: exitItem

            text: GuestStrings.menuExit
            icon.source: Images.exit
            position: MenuItem.Position.Last
            onTriggered: {
                GuestContent.onExitClicked();
                guestWindow.hide();
            }
        }
    }

    Qml.StackView {
        id: stack

        anchors.fill: parent
        replaceEnter: null
        replaceExit: null

        Component {
            id: initialPage

            BasePage {
                title: GuestStrings.logInOrSignUp
                spacing: 48
                leftButton {
                    text: OnboardingStrings.signUp
                    onClicked: {
                        LoginControllerAccess.guestWindowSignupClicked();
                    }
                }
                rightButton {
                    text: OnboardingStrings.login
                    onClicked: {
                        LoginControllerAccess.guestWindowLoginClicked();
                    }
                }
            }
        }

        Component {
            id: progressPage

            BasePage {
                description: content.description
                showProgressBar: true
                leftButton {
                    text: OnboardingStrings.signUp
                    enabled: false
                }
                rightButton {
                    text: OnboardingStrings.login
                    enabled: false
                }
                indeterminate: content.indeterminate
                progressValue: content.progressValue
            }
        }
    }

    Connections {
        target: LoginControllerAccess

        onLoginStarted: {
            if(!content.validating2FA) {
                content.indeterminate = true;
                content.description = OnboardingStrings.statusLogin;
                content.state = content.stateInProgress;
            }
        }

        onLoginFinished: (errorCode, errorMsg) => {
            switch(errorCode) {
                case ApiEnums.API_EMFAREQUIRED:
                    content.indeterminate = true;
                    content.description = OnboardingStrings.status2FA;
                    content.state = content.stateInProgress;
                    content.validating2FA = true;
                    break;
                case ApiEnums.API_EFAILED:
                case ApiEnums.API_EEXPIRED:
                    break;
                default:
                    content.validating2FA = false;
                    break;
            }
        }

        onFetchingNodesProgress: (progress) => {
            if(progress === 0.15) {
                content.indeterminate = false;
                content.description = OnboardingStrings.statusFetchNodes;
                content.state = content.stateInProgress;
            }
            content.progressValue = progress;
        }

        onFetchingNodesFinished: (firstTime) => {
            content.state = content.stateNormal;
            guestWindow.hide();
        }

        onLogout: {
            content.state = content.stateNormal;
        }

        onRegisterStarted: {
            content.indeterminate = true;
            content.description = OnboardingStrings.statusSignUp;
            content.state = content.stateInProgress;
        }

        onRegisterFinished: (success) => {
            if(!success) {
                content.state = content.stateNormal;
                return;
            }

            content.indeterminate = true;
            content.description = OnboardingStrings.statusWaitingForEmail;
            content.state = content.stateInProgress;
        }

        onEmailConfirmed: {
            content.state = content.stateNormal;
        }

        onAccountCreationResumed: {
            content.indeterminate = true;
            content.description = OnboardingStrings.statusWaitingForEmail;
            content.state = content.stateInProgress;
        }

        onAccountCreateCancelled: {
            content.state = content.stateNormal;
        }

        onLogin2FACancelled: {
            content.state = content.stateNormal;
        }

    }

    Component.onCompleted: {
        if(LoginControllerAccess.isAccountConfirmationResumed()) {
            content.indeterminate = true;
            content.description = OnboardingStrings.statusWaitingForEmail;
            content.state = content.stateInProgress;
        }
    }

}
