pragma Singleton
import QtQuick 2.15

QtObject {

    function isMac() {
        return Qt.platform.os === "osx";
    }

    function isWindows() {
        return Qt.platform.os === "windows";
    }

    function isLinux() {
        return Qt.platform.os === "linux";
    }

}
