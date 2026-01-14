import QtQuick 2.15

import common 1.0
import onboard 1.0
import QmlDeviceName 1.0

DeviceNamePageForm {
    id: root

    deviceNameTextField.text: deviceName.name

    function pageInWaitState(wait) {
        root.enabled = !wait;
        footerButtons.enabled = !wait;
        footerButtons.rightPrimary.icons.busyIndicatorVisible = wait;
    }

    signal deviceNameMoveToSyncType

    footerButtons.leftPrimary {
        text: Strings.skip
        onClicked: {
            window.close();
        }
    }

    footerButtons.rightPrimary.onClicked: {
        // i need to break the property binding, the call (checkDeviceName inside the getErrorCode) will trigger a update on the UserAttributes::DeviceName::requestDeviceName()
        // and will change the user entered deviceName and remove the error hint (if any) on the deviceNameTextField.
        deviceNameTextField.text = deviceNameTextField.text;

        let error = DeviceName.getErrorCode(deviceNameTextField.text);
        if (error === DeviceName.Error.NONE) {
            pageInWaitState(true);
            onboardingAccess.checkDeviceName(deviceNameTextField.text);
        }
        else {
            DeviceName.showErrorRoutine(deviceNameTextField, error);
        }
    }

    deviceNameTextField.onTextChanged: {
        if (deviceNameTextField.error) {
            DeviceName.showErrorRoutine(deviceNameTextField, DeviceName.Error.NONE)
        }
    }

    QmlDeviceName {
        id: deviceName

        onDeviceNameSet: {
            pageInWaitState(false);
            root.deviceNameMoveToSyncType();
        }
    }

    Connections {
        target: window

        function onInitializePageFocus() {
            deviceNameTextField.forceActiveFocus();
        }
    }

    Connections {
        target: onboardingAccess

        function onDeviceNameChecked(valid) {
            if (valid) {
                deviceName.setDeviceName(deviceNameTextField.text);
            }
            else {
                pageInWaitState(false);
                DeviceName.showErrorRoutine(deviceNameTextField, DeviceName.Error.NAME_EXIST);
            }
        }
    }
}

