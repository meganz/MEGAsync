import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.checkBoxes 1.0
import components.toolTips 1.0
import components.buttons 1.0 as Buttons
import components.textFields 1.0
import components.busyIndicator 1.0

import onboard 1.0

import BackupsModel 1.0
import ChooseLocalFolder 1.0

Item {
    id: root

    readonly property int totalHeight: 34
    readonly property int horizontalMargin: 8
    readonly property int internalMargin: 8
    readonly property int extraMarginWhenHintShowed: 5

    property bool editMode: false

    signal focusActivated

    anchors {
        right: parent != null ? parent.right : undefined
        left: parent != null ? parent.left : undefined
        rightMargin: horizontalMargin
        leftMargin: horizontalMargin
    }
    height: totalHeight

    Rectangle {
        id: background

        anchors {
            fill: parent
            rightMargin: internalMargin
            leftMargin: internalMargin
        }
        radius: internalMargin
        color: (index % 2 === 0) ? colorStyle.pageBackground : colorStyle.surface1

        Loader {
            id: content

            anchors.verticalCenter: parent.verticalCenter
            anchors.fill: parent
            sourceComponent: {
                if(!backupsProxyModel.selectedFilterEnabled
                        || error === backupsModelAccess.BackupErrorCode.NONE) {
                    return selectContent;
                }
                else {
                    if(error === backupsModelAccess.BackupErrorCode.SYNC_CONFLICT
                        || error === backupsModelAccess.BackupErrorCode.PATH_RELATION
                        || error === backupsModelAccess.BackupErrorCode.UNAVAILABLE_DIR
                        || error === backupsModelAccess.BackupErrorCode.SDK_CREATION) {
                        return conflictContent;
                    }
                    else {
                        // DUPLICATED_NAME or EXISTS_REMOTE errors
                        if(editMode) {
                            return editContent;
                        }
                        else {
                            return conflictContent;
                        }
                    }
                }
            }
        }
    }

    Component {
        id: selectContent

        Item {
            id: selectRoot

            readonly property int contentMargin: 8
            readonly property int checkboxWidth: 16
            readonly property int imageTextSpacing: 8
            readonly property int imageWidth: 16
            readonly property int textWidth: 248

            property int checkboxSpacing: checkbox.visible ? 16 : 0

            anchors {
                fill: parent
                margins: contentMargin
            }

            Row {
                id: leftSelectRow

                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                }
                spacing: selectRoot.checkboxSpacing

                CheckBox {
                    id: checkbox

                    anchors {
                        verticalCenter: parent.verticalCenter
                        topMargin: -checkbox.sizes.focusBorderWidth + 1
                    }
                    width: backupsProxyModel.selectedFilterEnabled ? 0 : selectRoot.checkboxWidth
                    checked: selected
                    visible: !backupsProxyModel.selectedFilterEnabled
                    manageChecked: true

                    Keys.onPressed: {
                        if (event.key === Qt.Key_Return
                                || event.key === Qt.Key_Enter
                                || event.key === Qt.Key_Space) {
                            selected = !selected;
                        }
                    }

                    onActiveFocusChanged: {
                        if(activeFocus)
                        {
                            focusActivated(index);
                        }
                    }
                }

                Row {
                    id: leftSelectInternalRow

                    anchors {
                        top: parent.top
                        bottom: parent.bottom
                    }
                    spacing: selectRoot.imageTextSpacing

                    Image {
                        id: selectImage

                        anchors.top: parent.top
                        height: selectRoot.imageWidth
                        width: selectRoot.imageWidth
                        source: done ? Images.checkCircle : Images.standard_DirIcon
                        sourceSize: Qt.size(selectRoot.imageWidth, selectRoot.imageWidth)
                    }

                    Texts.ElidedText {
                        id: selectText

                        anchors {
                            top: parent.top
                            bottom: parent.bottom
                            topMargin: 1
                        }
                        width: selectRoot.width - checkbox.width - selectRoot.checkboxSpacing
                               - selectRoot.imageTextSpacing - selectRoot.imageWidth -
                               (folderSize.visible ? (folderSize.width + selectRoot.checkboxSpacing) : 0) -
                               (busyIndicator.visible ? (busyIndicator.width + selectRoot.checkboxSpacing) : 0)
                        font.pixelSize: Texts.Text.Size.SMALL
                        text: name
                        color: colorStyle.textPrimary
                    }
                }

            } // Row: leftSelectRow

            Texts.SecondaryText {
                id: folderSize

                anchors {
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                }
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter
                text: size
                font.pixelSize: Texts.Text.Size.SMALL
                color: colorStyle.textSecondary
                visible: backupsProxyModel.selectedFilterEnabled && sizeReady
            }

            BusyIndicator {
                id: busyIndicator

                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                imageSize: Qt.size(12, 12)
                color: colorStyle.textAccent
                visible: backupsProxyModel.selectedFilterEnabled && !sizeReady
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: backupsProxyModel.selectedFilterEnabled
                             ? Qt.ArrowCursor
                             : Qt.PointingHandCursor
                onClicked: {
                    selected = !selected;
                }
                enabled: !backupsProxyModel.selectedFilterEnabled
            }

        } // Item: selectRoot

    } // Component: selectContent

    Component {
        id: conflictContent

        Item {
            id: conflictRoot

            readonly property int contentMargin: 8
            readonly property int imageTextSpacing: 8
            readonly property int imageWidth: 16
            readonly property int textWidth: 248
            readonly property int sizeTextWidth: 50

            property bool showChange: error === backupsModelAccess.BackupErrorCode.SYNC_CONFLICT
                                        || error === backupsModelAccess.BackupErrorCode.PATH_RELATION
                                        || error === backupsModelAccess.BackupErrorCode.UNAVAILABLE_DIR
                                        || error === backupsModelAccess.BackupErrorCode.SDK_CREATION

            Row {
                id: imageText

                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                    leftMargin: conflictRoot.contentMargin
                    topMargin: conflictRoot.contentMargin
                    bottomMargin: conflictRoot.contentMargin
                }
                spacing: imageTextSpacing

                SvgImage {
                    id: conflictImage

                    anchors.verticalCenter: parent.verticalCenter
                    source: error === backupsModelAccess.BackupErrorCode.SDK_CREATION
                            ? Images.alertCircle
                            : Images.alertTriangle
                    sourceSize: Qt.size(conflictRoot.imageWidth, conflictRoot.imageWidth)
                    color: error === backupsModelAccess.BackupErrorCode.SDK_CREATION
                           ? colorStyle.textError
                           : colorStyle.textWarning
                }

                Texts.ElidedText {
                    id: conflictText

                    anchors {
                        top: parent.top
                        bottom: parent.bottom
                    }
                    width: conflictRoot.width - conflictRoot.imageTextSpacing - conflictRoot.imageWidth
                            - buttonRow.width - conflictRoot.contentMargin
                    font.pixelSize: Texts.Text.Size.SMALL
                    text: name
                    color: error === backupsModelAccess.BackupErrorCode.SDK_CREATION
                           ? colorStyle.textError
                           : colorStyle.textWarning
                    showTooltip: false
                }
            }

            MouseArea {
                id: conflictMouseArea

                hoverEnabled: true
                anchors.fill: imageText

                ToolTip {
                    id: conflictTooltip

                    visible: parent.containsMouse
                    leftIconSource: Images.pc
                    text: folder
                    delay: 500
                    timeout: 5000

                    onVisibleChanged: {
                        if(visible) {
                            if((parent.mouseX + width) > 363) {
                                x = 363 - width;
                            }
                            else {
                                x = parent.mouseX;
                            }
                            y = parent.mouseY - height - 2;
                        }
                    }
                }
            }

            Item {
                id: buttonRow

                anchors {
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                }
                width: leftButton.width + removeButton.width - leftButton.sizes.focusBorderWidth

                Buttons.SecondaryButton {
                    id: leftButton

                    anchors {
                        right: removeButton.left
                        top: parent.top
                        bottom: parent.bottom
                        rightMargin: -sizes.focusBorderWidth
                    }
                    text: conflictRoot.showChange ? OnboardingStrings.changeFolder : OnboardingStrings.rename
                    sizes: Buttons.SmallSizes {}
                    icons {
                        position: Buttons.Icon.Position.LEFT
                        source: conflictRoot.showChange ? "" : Images.edit
                    }

                    onClicked: {
                        if(conflictRoot.showChange) {
                            folderDialog.openFolderSelector();
                        } else {
                            editMode = true;
                        }
                    }

                    ChooseLocalFolder {
                        id: folderDialog
                    }

                    Connections {
                        id: chooseLocalFolderConnection

                        target: folderDialog

                        function onFolderChoosen(folderPath) {
                            backupsModelAccess.change(folder, folderPath);
                        }
                    }
                }

                Buttons.SecondaryButton {
                    id: removeButton

                    anchors {
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                    }
                    icons.source: Images.trash
                    sizes: Buttons.SmallSizes {}

                    onClicked: {
                        backupsModelAccess.remove(folder);
                    }
                }

            } // Item: buttonRow

        } // Item: conflictRoot

    } // Component: conflictContent

    Component {
        id: editContent

        Row {
            id: editRow

            function doneAction() {
                editTextField.hint.visible = false;
                var error = backupsModelAccess.rename(folder, editTextField.text);
                switch(error) {
                    case backupsModelAccess.BackupErrorCode.NONE:
                    case backupsModelAccess.BackupErrorCode.SYNC_CONFLICT:
                    case backupsModelAccess.BackupErrorCode.PATH_RELATION:
                    case backupsModelAccess.BackupErrorCode.SDK_CREATION:
                        break;
                    case backupsModelAccess.BackupErrorCode.EXISTS_REMOTE:
                        editTextField.hint.visible = true;
                        editTextField.hint.text = OnboardingStrings.confirmBackupErrorRemote;
                        break;
                    case backupsModelAccess.BackupErrorCode.DUPLICATED_NAME:
                        editTextField.hint.visible = true;
                        editTextField.hint.text = OnboardingStrings.confirmBackupErrorDuplicated;
                        break;
                    default:
                        console.error("FolderRow: Unexpected error after rename -> " + error);
                        break;
                }
            }

            spacing: 2

            TextField {
                id: editTextField

                width: parent.width - parent.spacing - doneButton.width
                text: name
                leftIconSource: Images.edit
                leftIconColor: colorStyle.iconSecondary
                error: hint.visible
                sizes: SmallSizes {}
                validator: RegExpValidator { regExp: RegexExpressions.allowedFolderChars }

                onAccepted: {
                    doneAction()
                }

                onHeightChanged: {
                    if (editTextField.hint.visible) {
                        root.height = editTextField.height + root.extraMarginWhenHintShowed;
                    }
                    else {
                        root.height = root.totalHeight;
                    }
                }
            }

            Buttons.PrimaryButton {
                id: doneButton

                text: OnboardingStrings.done
                sizes: Buttons.SmallSizes {}

                onClicked: {
                    if (editTextField.acceptableInput){
                        doneAction()
                    }
                }
            }

            Connections {
                target: onboardingWindow

                function onLanguageChanged() {
                    if (editTextField.hint.visible) {
                        doneAction();
                    }
                }
            }
        } // Row: editRow

    } // Component: editContent

}
