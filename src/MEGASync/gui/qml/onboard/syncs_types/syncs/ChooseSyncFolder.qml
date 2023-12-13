import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3

import common 1.0

import components.buttons 1.0
import components.textFields 1.0

import onboard 1.0

import Syncs 1.0
import ChooseLocalFolder 1.0
import ChooseRemoteFolder 1.0

FocusScope {
    id: root

    property alias choosenPath: folderItem.text
    property bool local: true
    property alias folderField: folderItem

    readonly property int textEditMargin: 2

    width: parent.width
    height: folderItem.height
    Layout.preferredWidth: width
    Layout.preferredHeight: folderItem.height

    function reset() {
        if(!local) {
            remoteFolderChooser.reset();
        }
    }

    function getFolder() {
        var defaultFolder = "";

        if (local) {
            defaultFolder = localFolderChooser.getDefaultFolder(syncs.defaultMegaFolder);
        }
        else {
            defaultFolder = syncs.defaultMegaPath;
        }

        if ((local && !syncs.checkLocalSync(defaultFolder)) || (!local && !syncs.checkRemoteSync(defaultFolder))) {
            defaultFolder = "";
        }

        return defaultFolder;
    }

    Syncs {
        id: syncs
    }

    TextField {
        id: folderItem

        anchors {
            left: parent.left
            right: changeButtonItem.left
            top: parent.top
            rightMargin: textEditMargin
        }
        title: local ? OnboardingStrings.selectLocalFolder : OnboardingStrings.selectMEGAFolder
        text: getFolder()
        leftIconSource: local ? Images.pc : Images.megaOutline
        leftIconColor: enabled ? Styles.iconSecondary : Styles.iconDisabled
        textField.readOnly: true
        toolTip.leftIconSource: leftIconSource
        toolTip.timeout: 5000
    }

    OutlineButton {
        id: changeButtonItem

        height: folderItem.textField.height
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 15
        focus: true
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
            folderItem.text = folderPath;
        }
    }

    Connections {
        id: remoteFolderChooserConnection

        target: remoteFolderChooser
        enabled: !root.local

        function onFolderChoosen(remoteFolderPath) {
            folderItem.text = remoteFolderPath;
        }
    }

    ChooseLocalFolder {
        id: localFolderChooser
    }

    ChooseRemoteFolder {
        id: remoteFolderChooser
    }
}
