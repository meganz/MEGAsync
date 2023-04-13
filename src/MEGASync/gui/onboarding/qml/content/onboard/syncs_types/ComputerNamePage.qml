// C++
import Onboarding 1.0

ComputerNamePageForm {

    footerButtons {

        previousButton.visible: false

        notNowButton.visible: false

        nextButton.onClicked: {
            syncsFlow.state = syncType;
        }
    }

    computerNameTextField.textField.text: Onboarding.getComputerName();

}
