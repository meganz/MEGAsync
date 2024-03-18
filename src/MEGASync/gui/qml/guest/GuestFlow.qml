import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtGraphicalEffects 1.15

import common 1.0

import components.images 1.0
import components.buttons 1.0
import components.progressBars 1.0
import components.texts 1.0 as Texts

import onboard 1.0

import ApiEnums 1.0
import LoginController 1.0

Item {
    id: root

    readonly property string stateLoggedOut: "LOGGED_OUT"
    readonly property string stateInProgress: "IN_PROGRESS"
    readonly property string stateInProgressFetchNodes: "IN_PROGRESS_FETCH_NODES"
    readonly property string stateInProgressLoggingIn: "IN_PROGRESS_LOGGING"
    readonly property string stateInProgress2FA: "IN_PROGRESS_2FA"
    readonly property string stateInProgressCreatingAccount: "CREATING_ACCOUNT"
    readonly property string stateInProgressWaitingEmailConfirm: "WAITING_EMAIL_CONFIRMATION"
    readonly property string stateFetchNodesFinished: "FETCH_NODES_FINISHED"
    readonly property string stateBlocked: "BLOCKED"
    readonly property string stateInOnboarding: "IN_ONBOARDING"

    property string title: ""
    property bool indeterminate: true
    property double progressValue: 0.0

    function getState() {
        if(accountStatusControllerAccess.blockedState) {
            return root.stateBlocked;
        }
        else {
            switch(loginControllerAccess.state) {
                case LoginController.LOGGING_IN:
                    return root.stateInProgressLoggingIn;
                case LoginController.LOGGING_IN_2FA_REQUIRED:
                case LoginController.LOGGING_IN_2FA_VALIDATING:
                case LoginController.LOGGING_IN_2FA_FAILED:
                    return root.stateInProgress2FA;
                case LoginController.CREATING_ACCOUNT:
                    return root.stateInProgressCreatingAccount;
                case LoginController.WAITING_EMAIL_CONFIRMATION:
                case LoginController.CHANGING_REGISTER_EMAIL:
                    return root.stateInProgressWaitingEmailConfirm;
                case LoginController.FETCHING_NODES:
                case LoginController.FETCHING_NODES_2FA:
                    return root.stateInProgressFetchNodes;
                case LoginController.FETCH_NODES_FINISHED:
                    return root.stateFetchNodesFinished;
                case LoginController.FETCH_NODES_FINISHED_ONBOARDING:
                    return root.stateInOnboarding;
                default:
                    return root.stateLoggedOut;
            }
        }
    }

    state: getState()
    states: [
        State {
            name: root.stateLoggedOut
            StateChangeScript {
                script: view.replace(initialPageComponent);
            }
        },
        State {
            name: root.stateFetchNodesFinished
            extend: root.stateLoggedOut
            StateChangeScript {
                script: window.hide();
            }
        },
        State {
            name: root.stateInProgress
            StateChangeScript {
                script: view.replace(progressPageComponent);
            }
        },
        State {
            name: root.stateInProgressFetchNodes
            extend: root.stateInProgress
        },
        State {
            name: root.stateInProgressLoggingIn
            extend: root.stateInProgress
        },
        State {
            name: root.stateInProgressCreatingAccount
            extend: root.stateInProgress
        },
        State {
            name: root.stateInProgressWaitingEmailConfirm
            extend: root.stateInProgress
        },
        State {
            name: root.stateInProgress2FA
            extend: root.stateInProgress
        },
        State {
            name: root.stateBlocked
            StateChangeScript {
                script: view.replace(blockedPageComponent);
            }
        },
        State {
            name: root.stateInOnboarding
            StateChangeScript {
                script: view.replace(settingUpAccountPageComponent);
            }
        }
    ]

    SvgImage {
        id: image

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 16
        anchors.leftMargin: 16
        source: Images.mega
        sourceSize: Qt.size(24, 24)
    }

    IconButton {
        id: menuButton

        anchors{
            top: parent.top
            right: parent.right
            topMargin: 9
            rightMargin: 9
        }

        icons.source: Images.menu
        z: 3

        onClicked: {
            menuButton.checked = true;
            menu.visible = !menu.visible;
        }

        KeyNavigation.tab: menu.visible ? aboutMenuItem : view.currentItem.leftButton
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

        onVisibleChanged: {
            if (visible) {
                aboutMenuItem.forceActiveFocus();
            }
        }

        MenuItem {
            id: aboutMenuItem

            text: GuestStrings.menuAboutMEGA
            icon.source: Images.megaOutline
            position: MenuItem.Position.FIRST
            onTriggered: {
                guestContentAccess.onAboutMEGAClicked();
                window.hide();
            }
        }

        MenuItem {
            id: preferencesItem

            text: GuestStrings.menuSettings
            icon.source: Images.settings
            onTriggered: {
                guestContentAccess.onPreferencesClicked();
                window.hide();
            }
        }

        MenuItem {
            id: exitItem

            text: GuestStrings.menuExit
            icon.source: Images.exit
            position: MenuItem.Position.LAST
            onTriggered: {
                guestContentAccess.onExitClicked();
                window.hide();
            }

            KeyNavigation.tab: view.currentItem.leftButton
        }
    }

    Qml.StackView {
        id: view

        anchors.fill: parent
        replaceEnter: null
        replaceExit: null
    }

    Component {
        id: initialPageComponent

        BasePage {
            id: initialPage

            title: GuestStrings.logInOrSignUp
            spacing: 48
            showProgressBar: false
            leftButton {
                text: OnboardingStrings.signUp
                onClicked: {
                    loginControllerAccess.state = LoginController.SIGN_UP;
                    guestContentAccess.onInitialPageButtonClicked();
                }
            }
            rightButton {
                text: OnboardingStrings.login
                onClicked: {
                    loginControllerAccess.state = LoginController.LOGGED_OUT;
                    guestContentAccess.onInitialPageButtonClicked();
                }
            }

            Connections {
                target: window

                function onInitializePageFocus() {
                    leftButton.forceActiveFocus();
                }
            }
        }
    }

    Component {
        id: progressPageComponent

        BasePage {
            id: progressPage

            function getDescription() {
                switch(loginControllerAccess.state) {
                    case LoginController.LOGGING_IN:
                        return OnboardingStrings.statusLogin;
                    case LoginController.LOGGING_IN_2FA_REQUIRED:
                    case LoginController.LOGGING_IN_2FA_VALIDATING:
                    case LoginController.LOGGING_IN_2FA_FAILED:
                        return OnboardingStrings.status2FA;
                    case LoginController.CREATING_ACCOUNT:
                        return OnboardingStrings.statusSignUp;
                    case LoginController.WAITING_EMAIL_CONFIRMATION:
                    case LoginController.CHANGING_REGISTER_EMAIL:
                        return GuestStrings.statusWaitingForEmail;
                    case LoginController.FETCHING_NODES:
                    case LoginController.FETCHING_NODES_2FA:
                        return OnboardingStrings.statusFetchNodes;
                    default:
                        return "";
                }
            }

            description: getDescription()
            showProgressBar: true
            leftButton {
                text: OnboardingStrings.signUp
                enabled: false
            }
            rightButton {
                text: OnboardingStrings.login
                enabled: false
            }
            indeterminate: loginControllerAccess.progress === 0
            progressValue: loginControllerAccess.progress
            horizontalMargin: 48
        }
    }

    Component {
        id: blockedPageComponent

        BasePage {
            id: blockedPage

            showProgressBar: false
            imageSource: Images.warningGuest
            imageTopMargin: 110
            title: GuestStrings.accountTempLocked
            description: GuestStrings.accountTempLockedEmail
            descriptionFontSize: Texts.Text.Size.NORMAL
            descriptionColor: Styles.textPrimary
            descriptionLineHeight: 18
            leftButton {
                text: GuestStrings.logOut
                onClicked: {
                    guestContentAccess.onLogoutClicked();
                }
            }
            rightButton {
                text: GuestStrings.resendEmail
                icons {
                    source: Images.mail
                    position: Icon.Position.LEFT
                }
                onClicked: {
                    guestContentAccess.onVerifyEmailClicked();
                }
            }

            Connections {
                target: window

                function onInitializePageFocus() {
                    leftButton.forceActiveFocus();
                }
            }
        }
    }

    Component {
        id: settingUpAccountPageComponent

        BasePage {
            id: settingUpAccountPage

            showProgressBar: false
            title: GuestStrings.loggedInOnboarding
            imageSource: Images.settingUp
            leftButton.visible: false
            rightButton.visible: false
            spacing: 0
            bottomMargin: 150
        }
    }

    Connections {
        target: window

        function onHideRequested() {
            menu.close();
        }
    }
}
