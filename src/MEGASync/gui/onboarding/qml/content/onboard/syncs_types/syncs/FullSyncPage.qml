FullSyncPageForm {

    footerButtons{
        previousButton.onClicked: {
            syncsFlow.state = syncs;
        }
        nextButton.onClicked: {
            syncsFlow.state = finalState;
        }
    }

}
