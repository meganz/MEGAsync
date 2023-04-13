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

RowLayout {

    property bool local: true
    property url selectedUrl: selectedUrl
    property double selectedNode: selectedNode

    width: parent.width
    spacing: 8

    Custom.TextField {
        id: folderField

        Layout.preferredWidth: parent.width
        Layout.leftMargin: -folderField.textField.focusBorderWidth
        title: local ? OnboardingStrings.selectLocalFolder : OnboardingStrings.selectMEGAFolder
        text: "/MEGA"
        leftIcon.source: local
                         ? "../../../../../../images/onboarding/syncs/pc.svg"
                         : "../../../../../../images/onboarding/syncs/mega.svg"
    }

    Custom.Button {
        Layout.alignment: Qt.AlignBottom
        Layout.preferredHeight: folderField.textFieldRawHeight
        Layout.bottomMargin: folderField.textField.focusBorderWidth
        text: OnboardingStrings.choose
        onClicked: {
            fileDialog.open();
        }
    }

    FileDialog {
        id: fileDialog

        title: "Please choose a folder"
        folder: shortcuts.documents
        selectFolder: true
        onAccepted: {
            folderField.text = fileDialog.fileUrl.toString().slice(fileDialog.fileUrl.toString().lastIndexOf("/"));
            selectedUrl = fileDialog.fileUrl;
        }
    }
}
