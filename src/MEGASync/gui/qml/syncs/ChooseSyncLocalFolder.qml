import common 1.0

import syncs 1.0
import SyncsComponents 1.0

ChooseSyncFolderCore {
    id: root

    title: SyncsStrings.selectLocalFolder
    leftIconSource: Images.pc

    onButtonClicked:{
        syncsComponentAccess.clearLocalError();
        localFolderChooser.openFolderSelector(folderField.text);
    }

    function reset() {}

    function getFolder() {
        var defaultFolder = "";

        if(root.isOnboarding) {
            defaultFolder = localFolderChooser.getDefaultFolder(syncsDataAccess.defaultMegaFolder);
        }
        else { // Standalone syncs window
            if(syncsComponentAccess === null) {
                return defaultFolder;
            }

            if(syncsComponentAccess.remoteFolder === "") {
                defaultFolder = localFolderChooser.getDefaultFolder(syncsDataAccess.defaultMegaFolder);
            }
        }

        if (!syncsComponentAccess.checkLocalSync(defaultFolder)) {
            defaultFolder = "";
            syncsComponentAccess.clearLocalError();
        }

        return defaultFolder;
    }

    ChooseLocalFolder {
        id: localFolderChooser

        onFolderChoosen: (folderPath) => {
            folderField.text = folderPath;
        }
    }

}
