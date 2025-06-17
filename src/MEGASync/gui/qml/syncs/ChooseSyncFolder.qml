import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3

import common 1.0

import components.buttons 1.0
import components.textFields 1.0

import syncs 1.0
import SyncsComponents 1.0

FocusScope {
    id: root

    readonly property int textEditMargin: 2

    property alias chosenPath: folderItem.text
    property alias folderField: folderItem
    property alias title: folderItem.title
    property alias leftIconSource: folderItem.leftIconSource

    signal buttonClicked

    width: parent.width
    height: folderItem.height
    Layout.preferredWidth: width
    Layout.preferredHeight: folderItem.height

    TextField {
        id: folderItem

        anchors {
            left: parent.left
            right: changeButtonItem.left
            top: parent.top
            rightMargin: textEditMargin
        }
        leftIconColor: enabled ? ColorTheme.iconSecondary : ColorTheme.iconDisabled
        textField.readOnly: true
        toolTip {
            leftIconSource: leftIconSource
            timeout: 5000
        }
    }

    OutlineButton {
        id: changeButtonItem

        height: folderItem.textField.height
        anchors {
            right: parent.right
            top: parent.top
            topMargin: 15
        }
        focus: true
        text: Strings.choose
        onClicked: {
            root.buttonClicked();
        }
    }

    Connections {
        id: syncsConnection

        target: syncsDataAccess

        function onSyncRemoved() {
            syncsComponentAccess.updateDefaultFolders();
        }
    }
}
