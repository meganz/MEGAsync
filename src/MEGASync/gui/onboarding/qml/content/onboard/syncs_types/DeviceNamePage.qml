//Local
import Onboard 1.0

// C++
import QmlDeviceName 1.0

DeviceNamePageForm {

    footerButtons.rightPrimary.onClicked: {
        var emptyText = deviceNameTextField.text.length === 0;
        deviceNameTextField.error = emptyText;
        deviceNameTextField.hint.text = emptyText ? OnboardingStrings.errorEmptyDeviceName : "";
        deviceNameTextField.hint.visible = emptyText;

        if(emptyText) {
            return;
        }

        if(!deviceName.setDeviceName(deviceNameTextField.text)) {
            syncsPanel.state = syncType;
        }
    }

    deviceNameTextField.onTextChanged: {
        deviceNameTextField.error = false;
        deviceNameTextField.hint.text = "";
        deviceNameTextField.hint.visible = false;
    }

    QmlDeviceName {
        id: deviceName

        onDeviceNameSet: {
            syncsPanel.state = syncType;
        }
    }
}

