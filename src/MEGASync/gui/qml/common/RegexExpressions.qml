pragma Singleton
import QtQuick 2.12

QtObject {

    readonly property var digit2FA: /^[0-9]{6}$/
    readonly property var email: /\w+([-+.']\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*/
}
