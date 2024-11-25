import QtQuick 2.15

import common 1.0
import onboard 1.0
import QmlDeviceName 1.0

DeviceNamePageForm {
    id: root

    signal deviceNameMoveToSyncType

    footerButtons.leftPrimary {
        text: Strings.skip
        onClicked: {
            window.close();
        }
    }

    footerButtons.rightPrimary.onClicked: {

        let error = DeviceName.getErrorCode(onboardingAccess, deviceNameTextField.text);
        DeviceName.showErrorRoutine(deviceNameTextField, error);
        if (error !== DeviceName.Error.NONE) {
            return;
        }

        if(!deviceName.setDeviceName(deviceNameTextField.text)) {
            root.deviceNameMoveToSyncType();
        }
    }

    deviceNameTextField.onTextChanged: {
        if(deviceNameTextField.error) {
            DeviceName.showErrorRoutine(deviceNameTextField, DeviceName.Error.NONE)
        }
    }

    QmlDeviceName {
        id: deviceName

        onDeviceNameSet: {
            root.deviceNameMoveToSyncType();
        }
    }

    Connections {
        target: window

        function onInitializePageFocus() {
            deviceNameTextField.forceActiveFocus();
        }
    }
}

