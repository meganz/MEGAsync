SelectiveSyncPageForm {

    function getSyncData() {
        var localFolder = localFolderChooser.getSyncData();
        var handle = remoteFolderChooser.getSyncData();
        return { localFolder, handle };
    }

    footerButtons {

        previousButton.onClicked: {
            syncsFlow.state = syncs;
        }

        nextButton.onClicked: {
            console.log(getSyncData());
            syncsFlow.state = finalState;
        }
    }

}
