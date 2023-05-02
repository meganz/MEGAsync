import QtQuick 2.0
import QtQuick.Dialogs 1.3

Item {
    id: root

    function openFolderSelector() {
        console.info("onOpenFilePicker()");
        fileDialog.open();
    }

    function getHandle() {
        return selectedUrl;
    }

    function reset() {
        selectedUrl = "";
        folderChanged("");
    }

    property string textForTextField: "";
    property url selectedUrl: ""

    signal folderChanged(string folder);

    FileDialog {
        id: fileDialog

        title: "Please choose a folder"
        folder: shortcuts.documents
        selectFolder: true
        onAccepted: {
            textForTextField = fileDialog.fileUrl.toString().slice(fileDialog.fileUrl.toString().lastIndexOf("/"));
            selectedUrl = fileDialog.fileUrl
            root.folderChanged(textForTextField);
        }
    }
}
