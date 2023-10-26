// System
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3

// QML common
import Common 1.0
import Components.Buttons 1.0 as MegaButtons
import Components.TextFields 1.0 as MegaTextFields

// Local
import Syncs 1.0
import Onboard 1.0
import ChooseLocalFolder 1.0
import ChooseRemoteFolder 1.0

Item {
    id: root

    property alias choosenPath: folderItem.text
    property bool local: true
    property alias folderField: folderItem

    readonly property int textEditMargin: 2

    function reset() {
        if(!local) {
            remoteFolderChooser.reset();
        }
    }

    function getFolder() {
        var defaultFolder = "";

        if (local) {
            defaultFolder = localFolderChooser.getDefaultFolder(syncsCpp.defaultMegaFolder)
        }
        else {
            defaultFolder = syncsCpp.defaultMegaPath;
        }

        if ((local && !syncsCpp.checkLocalSync(defaultFolder)) || (!local && !syncsCpp.checkRemoteSync(defaultFolder)))
        {
            defaultFolder = ""
        }

        return defaultFolder;
    }

    Layout.preferredWidth: width
    Layout.preferredHeight: folderItem.height
    width: parent.width
    height: folderItem.height

    MegaTextFields.TextField {
        id: folderItem

        anchors.left: parent.left
        anchors.right: changeButton.left
        anchors.top: parent.top
        anchors.rightMargin: textEditMargin
        title: local ? OnboardingStrings.selectLocalFolder : OnboardingStrings.selectMEGAFolder
        text: getFolder()
        leftIconSource: local ? Images.pc : Images.megaOutline
        leftIconColor: enabled ? Styles.iconSecondary : Styles.iconDisabled
        textField.readOnly: true
        toolTip.leftIconSource: leftIconSource
        toolTip.timeout: 5000
    }

    MegaButtons.OutlineButton {
        id: changeButton

        height: folderItem.textField.height
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 15
        text: OnboardingStrings.choose
        onClicked: {
            folderItem.error = false;
            folderItem.hint.visible = false;

            if (local) {
                localFolderChooser.openFolderSelector(folderItem.text);
            }
            else {
                remoteFolderChooser.openFolderSelector();
            }
        }
    }

    Connections {
        id: localFolderChooserConnection

        target: localFolderChooser
        enabled: root.local

        function onFolderChoosen(folderPath) {
            folderItem.text = folderPath
        }
    }

    Connections {
        id: remoteFolderChooserConnection

        target: remoteFolderChooser
        enabled: !root.local

        function onFolderChoosen(remoteFolderPath) {
            folderItem.text = remoteFolderPath
        }
    }

    ChooseLocalFolder {
        id: localFolderChooser
    }

    ChooseRemoteFolder {
        id: remoteFolderChooser
    }
}
