import QtQml 2.12

//Local
import Onboard 1.0
import ComputerName 1.0

ComputerNamePageForm {

    property bool changingDeviceName: false

    footerButtons.nextButton.onClicked: {
        if(!computerName.setDeviceName(computerNameTextField.text)) {
            mainFlow.state = syncType;
            return;
        }
    }

    ComputerName {
        id: computerName

        onDeviceNameSet: {
                mainFlow.state = syncType;
        }
    }
}

