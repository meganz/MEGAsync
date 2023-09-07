// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Components.CheckBoxes 1.0 as MegaCheckBoxes
import Components.ToolTips 1.0 as MegaToolTips
import Components.Buttons 1.0 as MegaButtons
import Components.TextFields 1.0 as MegaTextFields

// Local
import Onboard 1.0

// C++
import BackupsModel 1.0
import ChooseLocalFolder 1.0

Rectangle {
    id: root

    readonly property int totalHeight: 32
    readonly property int horizontalMargin: 8
    readonly property int internalMargin: 8
    readonly property int extraMarginWhenHintShowed: 5

    property bool editMode: false

    height: totalHeight
    anchors.right: parent.right
    anchors.left: parent.left
    anchors.rightMargin: horizontalMargin
    anchors.leftMargin: horizontalMargin

    Rectangle {
        id: background

        anchors.right: root.right
        anchors.left: root.left
        anchors.top: root.top
        anchors.bottom: root.bottom
        anchors.rightMargin: internalMargin
        anchors.leftMargin: internalMargin
        radius: internalMargin
        color: (index % 2 === 0) ? "transparent" : Styles.surface2

        Loader {
            id: content

            anchors.verticalCenter: parent.verticalCenter
            anchors.fill: parent
            sourceComponent: {
                if(!backupsProxyModel.selectedFilterEnabled
                    || mError === BackupsModel.BackupErrorCode.None) {
                    return selectContent;
                } else {
                    if(mError === BackupsModel.BackupErrorCode.SyncConflict
                        || mError === BackupsModel.BackupErrorCode.PathRelation
                        || mError === BackupsModel.BackupErrorCode.UnavailableDir
                        || mError === BackupsModel.BackupErrorCode.SDKCreation) {
                        return conflictContent;
                    } else {
                        // DuplicatedName or ExistsRemote errors
                        if(editMode) {
                            return editContent;
                        } else {
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
            id: contentRoot

            readonly property int contentMargin: 8
            readonly property int checkboxWidth: 16
            readonly property int checkboxSpacing: 12
            readonly property int imageTextSpacing: 8
            readonly property int imageWidth: 16
            readonly property int textWidth: 248
            readonly property int sizeTextWidth: 50

            anchors.fill: parent
            anchors.margins: contentMargin

            Row {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                spacing: checkboxSpacing

                MegaCheckBoxes.CheckBox {
                    id: checkbox

                    width: backupsProxyModel.selectedFilterEnabled ? 0 : contentRoot.checkboxWidth
                    checked: mSelected
                    visible: !backupsProxyModel.selectedFilterEnabled
                }

                Row {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    spacing: contentRoot.imageTextSpacing

                    Image {
                        height: contentRoot.imageWidth
                        width: contentRoot.imageWidth
                        anchors.top: parent.top
                        source: Images.standard_DirIcon
                        sourceSize: Qt.size(contentRoot.imageWidth, contentRoot.imageWidth)
                        opacity: enabled ? 1.0 : 0.3
                    }

                    MegaTexts.Text {
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.topMargin: 1
                        width: contentRoot.width - checkbox.width - contentRoot.checkboxSpacing
                               - contentRoot.imageTextSpacing - contentRoot.imageWidth - contentRoot.sizeTextWidth
                        maximumLineCount: 1
                        font.pixelSize: MegaTexts.Text.Size.Small
                        wrapMode: Text.WrapAnywhere
                        text: mName
                        horizontalAlignment: Qt.AlignLeft
                        verticalAlignment: Qt.AlignVCenter
                    }
                }
            }

            MegaTexts.Text {
                width: contentRoot.sizeTextWidth
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                text: mSize
                font.pixelSize: MegaTexts.Text.Size.Small
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter
                maximumLineCount: 1
                wrapMode: Text.WrapAnywhere
                color: enabled ? Styles.textSecondary : Styles.textDisabled
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: backupsProxyModel.selectedFilterEnabled
                             ? Qt.ArrowCursor
                             : Qt.PointingHandCursor
                onClicked: {
                    mSelected = !mSelected;
                }
                enabled: !backupsProxyModel.selectedFilterEnabled
            }
        }
    }

    Component {
        id: conflictContent

        Item {
            id: contentRoot

            readonly property int contentMargin: 8
            readonly property int imageTextSpacing: 8
            readonly property int imageWidth: 16
            readonly property int textWidth: 248
            readonly property int sizeTextWidth: 50

            property bool showChange: mError === BackupsModel.BackupErrorCode.SyncConflict
                                        || mError === BackupsModel.BackupErrorCode.PathRelation
                                        || mError === BackupsModel.BackupErrorCode.UnavailableDir
                                        || mError === BackupsModel.BackupErrorCode.SDKCreation

            Row {
                id: imageText

                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.leftMargin: contentRoot.contentMargin
                anchors.topMargin: contentRoot.contentMargin
                anchors.bottomMargin: contentRoot.contentMargin
                spacing: imageTextSpacing

                MegaImages.SvgImage {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    source: mError === BackupsModel.BackupErrorCode.SDKCreation
                            ? Images.alertCircle
                            : Images.alertTriangle
                    sourceSize: Qt.size(contentRoot.imageWidth, contentRoot.imageWidth)
                    color: mError === BackupsModel.BackupErrorCode.SDKCreation
                           ? Styles.textError
                           : Styles.textWarning
                }

                MegaTexts.Text {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: contentRoot.width - contentRoot.imageTextSpacing - contentRoot.imageWidth
                            - buttonRow.width - 8
                    maximumLineCount: 1
                    font.pixelSize: MegaTexts.Text.Size.Small
                    wrapMode: Text.WrapAnywhere
                    text: mName
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter
                    color: mError === BackupsModel.BackupErrorCode.SDKCreation
                           ? Styles.textError
                           : Styles.textWarning
                }
            }
            MouseArea {
                hoverEnabled: true
                anchors.fill: imageText

                MegaToolTips.ToolTip {
                    visible: parent.containsMouse
                    leftIconSource: Images.pc
                    text: mFolder
                    delay: 500
                    timeout: 5000

                    onVisibleChanged: {
                        if(visible) {
                            if((parent.mouseX + width) > 363) {
                                x = 363 - width;
                            } else {
                                x = parent.mouseX;
                            }
                            y = parent.mouseY - height - 2;
                        }
                    }
                }
            }

            Row {
                id: buttonRow

                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                MegaButtons.SecondaryButton {
                    text: contentRoot.showChange ? OnboardingStrings.changeFolder : OnboardingStrings.rename
                    icons.position: MegaButtons.Icon.Position.LEFT
                    icons.source: contentRoot.showChange ? "" : Images.edit
                    onClicked: {
                        if(contentRoot.showChange) {
                            folderDialog.openFolderSelector();
                        } else {
                            editMode = true;
                        }
                    }
                    sizes: MegaButtons.SmallSizes {}

                    ChooseLocalFolder {
                        id: folderDialog

                        onFolderChanged: {
                            BackupsModel.change(mFolder, folderDialog.getFolder());
                        }
                    }
                }

                MegaButtons.SecondaryButton {
                    icons.source: Images.trash
                    onClicked: {
                        BackupsModel.remove(mFolder);
                    }
                    sizes: MegaButtons.SmallSizes {}
                }
            }

        }
    }

    Component {
        id: editContent

        Row {
            spacing: 2

            MegaTextFields.TextField {
                id: editTextField

                width: parent.width - parent.spacing - doneButton.width
                text: mName
                leftIcon.source: Images.edit
                leftIcon.color: Styles.iconSecondary
                error: hint.visible
                sizes: MegaTextFields.SmallSizes {}
            }

            MegaButtons.PrimaryButton {
                id: doneButton

                text: OnboardingStrings.done
                sizes: MegaButtons.SmallSizes {}
                onClicked: {
                    editTextField.hint.visible = false;
                    var error = BackupsModel.rename(mFolder, editTextField.text);
                    switch(error) {
                        case BackupsModel.BackupErrorCode.None:
                        case BackupsModel.BackupErrorCode.SyncConflict:
                        case BackupsModel.BackupErrorCode.PathRelation:
                        case BackupsModel.BackupErrorCode.SDKCreation:
                            root.height = root.totalHeight;
                            break;
                        case BackupsModel.BackupErrorCode.ExistsRemote:
                            editTextField.hint.text =
                                OnboardingStrings.confirmBackupErrorRemote.replace(RegexExpressions.betweenCommas,
                                                                                   '"' + editTextField.text + '"');
                            editTextField.hint.visible = true;
                            root.height = editTextField.height + root.extraMarginWhenHintShowed;
                            break;
                        case BackupsModel.BackupErrorCode.DuplicatedName:
                            editTextField.hint.text =
                                OnboardingStrings.confirmBackupErrorDuplicated.replace(RegexExpressions.betweenCommas,
                                                                                       '"' + editTextField.text + '"');
                            editTextField.hint.visible = true;
                            root.height = editTextField.height + root.extraMarginWhenHintShowed;
                            break;
                        default:
                            root.height = root.totalHeight;
                            console.error("FolderRow: Unexpected error after rename -> " + error);
                            break;
                    }
                }
            }
        }
    }

}
