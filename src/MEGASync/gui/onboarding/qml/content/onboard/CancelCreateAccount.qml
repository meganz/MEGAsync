//import QtQuick.Window 2.12
//import QtQuick 2.12
//import QtQuick.Layouts 1.12

//// QML common
//import Common 1.0

//// Local
import Onboard 1.0
//import Components.Texts 1.0 as MegaTexts
//import Components.Buttons 1.0 as MegaButtons

ConfirmCloseDialog {
    id: dialog

    titleText: OnboardingStrings.cancelAccountCreationTitle
    bodyText: OnboardingStrings.cancelAccountCreationBody
    cancelButtonText: OnboardingStrings.cancelAccountCancelButton
    acceptButtonText: OnboardingStrings.cancelAccountAcceptButton
}
