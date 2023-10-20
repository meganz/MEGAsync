//import QtQuick.Window 2.15
//import QtQuick 2.15
//import QtQuick.Layouts 1.15

//// QML common
//import Common 1.0

//// Local
import Onboard 1.0
//import Components.Texts 1.0 as MegaTexts
//import Components.Buttons 1.0 as MegaButtons

ConfirmCloseDialog {
    id: dialog

    titleText: OnboardingStrings.cancelLoginTitle
    bodyText: OnboardingStrings.cancelLoginBodyText
    cancelButtonText: OnboardingStrings.cancelLoginSecondaryButton
    acceptButtonText: OnboardingStrings.cancelLoginPrimaryButton
}
