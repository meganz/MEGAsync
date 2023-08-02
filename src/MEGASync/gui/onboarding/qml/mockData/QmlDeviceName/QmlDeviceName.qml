import QtQuick 2.0

Item {
    id: root

    property string mName: "My Device"

    signal deviceNameSet

    function getDeviceName() {
        console.debug("mockup QmlDeviceName::getDeviceName()");
        return "My PC name";
    }

    function setDeviceName(deviceName) {
        console.debug("mockup QmlDeviceName::setDeviceName() : deviceName -> " + deviceName);
        deviceNameSet();
        return false;
    }

}
