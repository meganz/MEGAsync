// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
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

    property alias choosenPath: folderField.text
    property alias remoteTest: remoteFolderChooser
    property bool local: true
    property url selectedUrl: selectedUrl
    property double selectedNode: selectedNode
    property bool isValid: true
    property alias folderField: folderField

    readonly property int textEditMargin: 2

    function reset() {
        if(!local) {
            remoteFolderChooser.reset();
        }
    }

    function getFolder() {
        var defaultFolder = "";

        if (local) {
            defaultFolder = localFolderChooser.getDefaultFolder(syncsCpp.DEFAULT_MEGA_FOLDER)
        }
        else {
            defaultFolder = syncsCpp.DEFAULT_MEGA_PATH;
        }

        if ((local && !syncsCpp.checkLocalSync(defaultFolder)) || (!local && !syncsCpp.checkRemoteSync(defaultFolder)))
        {
            defaultFolder = ""
        }

        return defaultFolder;
    }

    Layout.preferredWidth: width
    Layout.preferredHeight: folderField.height
    width: parent.width
    height: folderField.height

    MegaTextFields.TextField {
        id: folderField

        anchors.left: parent.left
        anchors.right: changeButton.left
        anchors.top: parent.top
        anchors.rightMargin: textEditMargin
        title: local ? OnboardingStrings.selectLocalFolder : OnboardingStrings.selectMEGAFolder
        text: getFolder()
        leftIcon.source: local ? Images.pc : Images.megaOutline
        leftIcon.color: enabled ? Styles.iconSecondary : Styles.iconDisabled
        textField.readOnly: true
        toolTip.leftIconSource: leftIcon.source
        toolTip.timeout: 5000
    }

    MegaButtons.OutlineButton {
        id: changeButton

        height: folderField.textField.height
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 15
        text: OnboardingStrings.choose
        onClicked: {
            folderField.error = false;
            folderField.hint.visible = false;

            if (local) {
                localFolderChooser.openFolderSelector(folderField.text);
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

        function onFolderChoosen(folder) {
            folderField.text = folder
        }
    }

    Connections {
        id: remoteFolderChooserConnection

        target: remoteFolderChooser
        enabled: !root.local

        function onFolderChoosen(folder) {
            folderField.text = folder
        }
    }

    ChooseLocalFolder {
        id: localFolderChooser
    }

    ChooseRemoteFolder {
        id: remoteFolderChooser
    }
}
