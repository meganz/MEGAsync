import QtQml 2.12

//Local
import Onboard 1.0
import ComputerName 1.0

ComputerNamePageForm {

    footerButtons.rightPrimary.onClicked: {
        if(!computerName.setDeviceName(computerNameTextField.text)) {
            syncsPanel.state = syncType;
            return;
        }
    }

    ComputerName {
        id: computerName

        onDeviceNameSet: {
                syncsPanel.state = syncType;
        }
    }
}

