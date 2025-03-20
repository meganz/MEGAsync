import common 1.0

import syncs 1.0
import SyncsComponents 1.0

ChooseSyncFolderCore {
    id: root

    title: SyncsStrings.selectMEGAFolder
    leftIconSource: Images.megaOutline
    choosenPath: syncsComponentAccess.getInitialRemoteFolder()

    onButtonClicked: {
        syncsComponentAccess.chooseRemoteFolderButtonClicked();
        remoteFolderChooser.openFolderSelector();
    }

    ChooseRemoteFolder {
        id: remoteFolderChooser

        onFolderChoosen: (folderPath) => {
            folderField.text = folderPath;
        }
    }

}
