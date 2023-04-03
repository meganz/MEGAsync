import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.3

import Common 1.0
import Components 1.0 as Custom

ColumnLayout {

    property bool local: true
    property url selectedUrl: selectedUrl
    property double selectedNode: selectedNode

    spacing: 8

    Text {
        id: title
        Layout.alignment: Qt.AlignLeft
        text: local ? qsTr("Select a local folder") : qsTr("Select a MEGA folder")
        color: Styles.textColor
        font.pixelSize: 14
        font.bold: true
    }

    RowLayout{
        spacing: 8

        Custom.IconTextField {
            id: textField

            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            text: "/MEGA"
            imageSource: local ? "../../../../../../images/onboarding/syncs/pc.svg"
                               : "../../../../../../images/onboarding/syncs/mega.svg"
        }

        Custom.Button {
            id: button

            Layout.topMargin: textField.outRect.border.width
            Layout.alignment: Qt.AlignTop
            text: qsTr("Choose")
            onClicked:
            {
                fileDialog.open();
            }
        }
    }

    FileDialog {
        id: fileDialog

        title: "Please choose a folder"
        folder: shortcuts.documents
        selectFolder: true
        onAccepted: {
            console.log("You chose: " + fileDialog.fileUrl)
            console.log(QDir.toNativeSeparators(fileDialog.fileUrl))
            textField.text = fileDialog.fileUrl
            selectedUrl = fileDialog.fileUrl
        }
    }
}
