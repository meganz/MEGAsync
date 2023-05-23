import QtQml 2.12

//Local
import Onboard 1.0

// C++
import Onboarding 1.0

ComputerNamePageForm {

    property bool changingDeviceName: false

    footerButtons.nextButton.onClicked: {
        changingDeviceName = Onboarding.setDeviceName(computerNameTextField.text);
        if(!changingDeviceName) {
            syncsFlow.state = syncType;
            return;
        }
        computerNameTextField.textField.enabled = false;
    }

    Connections {
        target: Onboarding

        onDeviceNameReady: {
            computerNameTextField.text = deviceName;
            computerNameTextField.textField.enabled = true;
            if(changingDeviceName) {
                syncsFlow.state = syncType;
            }
        }
    }

    Component.onCompleted: {
        Onboarding.getComputerName();
    }
}

