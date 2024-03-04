import QtQuick 2.15

import common 1.0
import onboard 1.0
import QmlDeviceName 1.0

DeviceNamePageForm {
    id: root

    signal deviceNameMoveToSyncType

    footerButtons.rightPrimary.onClicked: {
        var emptyText = deviceNameTextField.text.length === 0;
        if(emptyText) {
            deviceNameTextField.hint.textColor = colorStyle.textError;
        }
        deviceNameTextField.error = emptyText;
        deviceNameTextField.hint.text = emptyText ? OnboardingStrings.errorEmptyDeviceName : "";
        deviceNameTextField.hint.visible = emptyText;

        if(emptyText) {
            return;
        }

        if(!deviceName.setDeviceName(deviceNameTextField.text)) {
            root.deviceNameMoveToSyncType();
        }
    }

    deviceNameTextField.onTextChanged: {
        deviceNameTextField.error = false;
        deviceNameTextField.hint.text = "";
        deviceNameTextField.hint.visible = false;

        if(deviceNameTextField.text.length >= deviceNameTextField.textField.maximumLength) {
            deviceNameTextField.hint.textColor = colorStyle.textSecondary;
            deviceNameTextField.hint.text = OnboardingStrings.errorDeviceNameLimit;
            deviceNameTextField.hint.visible = true;
        }
    }

    QmlDeviceName {
        id: deviceName

        onDeviceNameSet: {
            root.deviceNameMoveToSyncType();
        }
    }

    Connections {
        target: onboardingWindow

        function onInitializePageFocus() {
            deviceNameTextField.forceActiveFocus();
        }
    }
}

