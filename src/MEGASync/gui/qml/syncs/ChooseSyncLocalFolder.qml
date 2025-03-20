import common 1.0

import syncs 1.0
import SyncsComponents 1.0

ChooseSyncFolderCore {
    id: root

    title: SyncsStrings.selectLocalFolder
    leftIconSource: Images.pc
    choosenPath: syncsComponentAccess.getInitialLocalFolder()

    onButtonClicked:{
        syncsComponentAccess.chooseLocalFolderButtonClicked();
        localFolderChooser.openFolderSelector(folderField.text);
    }

    ChooseLocalFolder {
        id: localFolderChooser

        onFolderChoosen: (folderPath) => {
            folderField.text = folderPath;
        }
    }

}
