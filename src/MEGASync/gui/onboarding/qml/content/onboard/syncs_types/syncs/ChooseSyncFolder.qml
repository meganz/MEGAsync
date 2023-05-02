// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.3

// QML common
import Common 1.0
import Components 1.0 as Custom

// Local
import Onboard 1.0
import ChooseLocalFolder 1.0
import ChooseRemoteFolder 1.0

Rectangle {

    function getSyncData() {
        return local ? localFolderChooser.getFolder() : remoteFolderChooser.getHandle();
    }

    function folderSelectionChanged(folder) {
        isValid = folder.length;
        folderField.text = isValid ? folder : "/MEGA";
    }

    function reset() {
        local ? localFolderChooser.reset() : remoteFolderChooser.reset();
    }

    property bool local: true
    property url selectedUrl: selectedUrl
    property double selectedNode: selectedNode
    property bool isValid: false

    readonly property int buttonWidth: 85
    readonly property int buttonHeight: 36
    readonly property int textEditMargin: 8

    width: parent.width
    height: folderField.height
    color: "transparent"

    Custom.TextField {
        id: folderField

        anchors.left: parent.left
        anchors.right: changeButton.left
        anchors.top: parent.top
        anchors.rightMargin: textEditMargin
        title: local ? OnboardingStrings.selectLocalFolder : OnboardingStrings.selectMEGAFolder
        text: "/MEGA"
        leftIconSource: local ? Images.pc : Images.mega
        leftIconColor: Styles.iconSecondary
    }

    Custom.Button {
        id: changeButton

        width: buttonWidth
        height: buttonHeight
        anchors.right: parent.right
        anchors.bottom: folderField.bottom
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
