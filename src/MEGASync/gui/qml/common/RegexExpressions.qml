pragma Singleton
import QtQuick 2.15

QtObject {

    readonly property var digit2FA: /^[0-9]$/
    readonly property var allDigits2FA: /^[0-9]{6}$/
    readonly property var email: /^[a-zA-Z0-9.!#$%&'*+\/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\.[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)+$/
    readonly property var upperCaseLeters: /[A-Z]/
    readonly property var lowerCaseLeters: /[a-z]/
    readonly property var numbers: /\d/
    readonly property var specialCharacters: /[!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]+/
    readonly property var betweenCommas: /"([^"]*)"/g
    readonly property var allowedFolderChars: /^(?!\s)[^*><?"\/\\\|:]{1,255}$/
    readonly property var allow3DigitsOnly: /^(\d{0,3}(?:\.\d{0,2})?)?$/
}
