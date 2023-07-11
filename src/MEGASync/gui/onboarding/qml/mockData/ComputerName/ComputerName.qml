import QtQuick 2.0
import Onboard 1.0

Item {
    id: root

    property string deviceName: OnboardingStrings.myComputer

    signal deviceNameSet

    function getDeviceName() {
        console.debug("mockup ComputerName::getComputerName()");
        return "My PC name";
    }

    function setDeviceName(deviceName) {
        console.debug("mockup ComputerName::setDeviceName() : deviceName -> " + deviceName);
        deviceNameSet();
        return false;
    }

}
