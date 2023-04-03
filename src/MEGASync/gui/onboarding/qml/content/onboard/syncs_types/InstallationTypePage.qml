InstallationTypePageForm {

    buttonGroup.onCheckStateChanged: {
        if(buttonGroup.checkedButton != null) {
            footerButtons.nextButton.enabled = true;
        }
    }

    footerButtons.nextButton.enabled: false

}
