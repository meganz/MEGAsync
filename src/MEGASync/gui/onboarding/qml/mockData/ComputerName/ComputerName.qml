import QtQuick 2.0
import Onboard 1.0

Item {
    id: root
    property string deviceName: OnboardingStrings.myComputer;

    function getDeviceName() {
        console.info("getComputerName()");
        return "My PC name";
    }

    function setDeviceName(deviceName) {
        console.info("setDeviceName(deviceName)" + deviceName)
        return false;
    }
}
