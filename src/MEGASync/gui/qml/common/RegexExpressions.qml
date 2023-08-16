pragma Singleton
import QtQuick 2.12

QtObject {

    readonly property var digit2FA: /^[0-9]{6}$/
    readonly property var email: /\w+([-+.']\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*/
    readonly property var upperCaseLeters: /[A-Z]/
    readonly property var lowerCaseLeters: /[a-z]/
    readonly property var numbers: /\d/
    readonly property var specialCharacters: /[!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]+/
    readonly property var betweenCommas: /"([^"]*)"/g

}
