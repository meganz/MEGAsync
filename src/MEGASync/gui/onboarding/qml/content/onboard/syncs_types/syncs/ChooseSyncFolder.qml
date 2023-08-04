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
import Onboard 1.0
import ChooseLocalFolder 1.0
import ChooseRemoteFolder 1.0

Rectangle {

    property bool local: true
    property url selectedUrl: selectedUrl
    property double selectedNode: selectedNode
    property bool isValid: false
    property alias folderField: folderField

    readonly property int textEditMargin: 2

    function getSyncData() {
        return local ? localFolderChooser.getFolder() : remoteFolderChooser.getHandle();
    }

    function folderSelectionChanged(folder) {
        isValid = folder.length;
        folderField.text = isValid ? folder : "/MEGA";
    }

    function reset() {
        if(local) {
            localFolderChooser.reset();
        } else {
            remoteFolderChooser.reset();
        }
    }

    width: parent.width
    height: folderField.height
    color: "transparent"

    MegaTextFields.TextField {
        id: folderField

        anchors.left: parent.left
        anchors.right: changeButton.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.rightMargin: textEditMargin
        title: local ? OnboardingStrings.selectLocalFolder : OnboardingStrings.selectMEGAFolder
        text: "/MEGA"
        leftIcon.source: local ? Images.pc : Images.megaOutline
        leftIcon.color: Styles.iconSecondary
        textField.readOnly: true
        hint.icon: Images.alertTriangle
    }

    MegaButtons.OutlineButton {
        id: changeButton

        height: folderField.textField.height
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        text: OnboardingStrings.change
        onClicked: {
            var folderChooser = local ? localFolderChooser : remoteFolderChooser;
            folderChooser.openFolderSelector();
        }
    }

    ChooseLocalFolder {
        id: localFolderChooser

        onFolderChanged: {
            folderSelectionChanged(folder);
        }
    }

    ChooseRemoteFolder {
        id: remoteFolderChooser

        onFolderChanged: {
            folderSelectionChanged(folder);
        }
    }
}
