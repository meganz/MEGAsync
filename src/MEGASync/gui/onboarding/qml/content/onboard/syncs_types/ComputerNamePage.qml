import QtQml 2.12

//Local
import Onboard 1.0
import ComputerName 1.0

ComputerNamePageForm {

    property bool changingDeviceName: false

    footerButtons.nextButton.onClicked: {
        computerName.setDeviceName(computerNameTextField.text);
        if(!computerName.requestingDeviceName) {
            mainFlow.state = syncType;
            return;
        }
    }

    ComputerName {
        id: computerName

        onDeviceNameChanged: {
            if(!computerName.requestingDeviceName) {
                mainFlow.state = syncType;
            }
        }
    }
}

