import common 1.0

import syncs 1.0
import SyncsComponents 1.0

ChooseSyncFolderCore {
    id: root

    title: SyncsStrings.selectMEGAFolder
    leftIconSource: Images.megaOutline

    onButtonClicked: {
        syncsComponentAccess.clearRemoteError();
        remoteFolderChooser.openFolderSelector();
    }

    function reset() {
        remoteFolderChooser.reset();
    }

    function getFolder() {
        var defaultFolder = "";

        if (root.isOnboarding) {
            defaultFolder = syncsDataAccess.defaultMegaPath;
        }
        else { // Standalone syncs window
            if(syncsComponentAccess === null) {
                return defaultFolder;
            }

            if(syncsComponentAccess.remoteFolder === "") {
                defaultFolder = syncsDataAccess.defaultMegaPath;
            }
            else {
                defaultFolder = syncsComponentAccess.remoteFolder;
            }
        }

        if (!syncsComponentAccess.checkRemoteSync(defaultFolder)) {
            defaultFolder = "";
            syncsComponentAccess.clearRemoteError();
        }

        return defaultFolder;
    }

    ChooseRemoteFolder {
        id: remoteFolderChooser

        onFolderChoosen: (folderPath) => {
            folderField.text = folderPath;
        }
    }

}
