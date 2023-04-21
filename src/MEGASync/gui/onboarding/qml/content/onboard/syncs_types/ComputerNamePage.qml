import QtQml 2.12

// C++
import Onboarding 1.0

ComputerNamePageForm {

    property bool changingDeviceName: false;

    footerButtons {
        previousButton.visible: false
        notNowButton.visible: false
        nextButton.onClicked: {
            changingDeviceName = Onboarding.setDeviceName(computerNameTextField.text);
            if(!changingDeviceName)
            {
                syncsFlow.state = syncType;
                return;
            }
            computerNameTextField.textField.enabled = false;
        }
    }

    computerNameTextField.textField{
        enabled: false;
        text: Onboarding.getComputerName();
    }

    Connections{
        target: Onboarding
        onDeviceNameReady:{
            computerNameTextField.text = deviceName;
            computerNameTextField.textField.enabled = true;
            if(changingDeviceName)
            {
                syncsFlow.state = syncType;
            }
        }
    }
}

