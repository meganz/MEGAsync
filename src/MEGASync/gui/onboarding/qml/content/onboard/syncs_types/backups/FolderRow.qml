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

Rectangle {
    id: root

    readonly property int totalHeight: 32
    readonly property int horizontalMargin: 8
    readonly property int internalMargin: 8

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
                if(backupsProxyModel.selectedFilterEnabled) {
                    if(mError !== BackupsModel.BackupErrorCode.DuplicatedName
                            && mError !== BackupsModel.BackupErrorCode.ExistsRemote) {
                        return confirmContent;
                    } else {
                        return conflictContent;
                    }
                } else {
                    return selectContent;
                }
            }
        }
    }

    Component {
        id: selectContent

        Item {
            anchors.fill: parent

            RowLayout {
                anchors.fill: parent

                RowLayout {
                    Layout.alignment: Qt.AlignLeft

                    MegaCheckBoxes.CheckBox {
                        Layout.leftMargin: 8
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                        checked: mSelected
                        checkable: mSelectable
                        enabled: mSelectable
                    }

                    MegaImages.SvgImage {
                        Layout.leftMargin: 18
                        source: mError ? Images.alertTriangle : Images.folder
                        sourceSize: Qt.size(14, 14)
                        color: mError ? Styles.textWarning : color
                        opacity: mSelectable ? 1.0 : 0.3
                    }

                    MegaTexts.Text {
                        Layout.leftMargin: 13
                        Layout.maximumWidth: 345
                        maximumLineCount: 1
                        wrapMode: Text.WrapAnywhere
                        text: mName
                    }
                }

                MegaTexts.Text {
                    Layout.alignment: Qt.AlignRight
                    Layout.rightMargin: 8
                    text: mSize
                    font.pixelSize: MegaTexts.Text.Size.Small
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    mSelected = !mSelected;
                }
            }
        }
    }

    Component {
        id: confirmContent

        RowLayout {

            RowLayout {
                Layout.alignment: Qt.AlignLeft

                MegaImages.SvgImage {
                    Layout.leftMargin: 8
                    Layout.preferredWidth: 16
                    source: mError !== 0 ? Images.alertTriangle : Images.folder
                    sourceSize: Qt.size(14, 14)
                    color: mError ? Styles.textWarning : color
                    opacity: mSelectable ? 1.0 : 0.3
                }

                MegaTexts.Text {
                    Layout.leftMargin: 13
                    Layout.maximumWidth: 345
                    Layout.preferredWidth: 345
                    maximumLineCount: 1
                    wrapMode: Text.WrapAnywhere
                    text: mName
                }
            }

            MegaTexts.Text {
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 8
                text: mSize
                font.pixelSize: MegaTexts.Text.Size.Small
            }
        }
    }

    Component {
        id: conflictContent

        RowLayout {

            RowLayout {
                Layout.alignment: Qt.AlignLeft
                spacing: 8

                MegaImages.SvgImage {
                    Layout.leftMargin: 8
                    source: Images.alertTriangle
                    sourceSize: Qt.size(14, 14)
                    color: Styles.textWarning
                }

                MegaTexts.Text {
                    Layout.maximumWidth: 300
                    Layout.preferredWidth: 300
                    maximumLineCount: 1
                    wrapMode: Text.WrapAnywhere
                    text: mName

                    MouseArea {
                        id: conflictMouseArea

                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            content.sourceComponent = editContent;
                        }
                    }
                }

                MegaToolTips.ToolTip {
                    visible: conflictMouseArea.containsMouse
                    leftIconSource: Images.pc
                    text: toolTip
                    delay: 500
                    timeout: 5000
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 4

                MegaButtons.SecondaryButton {
                    Layout.preferredHeight: 26
                    Layout.preferredWidth: 87
                    text: OnboardingStrings.rename
                    icons.position: MegaButtons.Icon.Position.LEFT
                    icons.source: Images.edit
                    onClicked: {
                        content.sourceComponent = editContent;
                    }
                }

                MegaButtons.SecondaryButton {
                    Layout.preferredHeight: 26
                    Layout.rightMargin: 8
                    icons.source: Images.trash
                    onClicked: {
                        BackupsModel.remove(mFolder);
                    }
                }
            }
        }
    }

    Component {
        id: editContent

        RowLayout {
            MegaTextFields.TextField {
                id: editTextField

                Layout.preferredWidth: 313
                text: mName
                leftIcon.source: Images.edit
                leftIcon.color: Styles.iconSecondary
                error: mErrorVisible
                hint.visible: mErrorVisible
                hint.text: {
                    if(mError === BackupsModel.BackupErrorCode.ExistsRemote) {
                        return OnboardingStrings.confirmBackupErrorRemote;
                    }
                    if(mError === BackupsModel.BackupErrorCode.DuplicatedName) {
                        return OnboardingStrings.confirmBackupErrorDuplicated;
                    }
                    return "";
                }
            }

            MegaButtons.PrimaryButton {
                Layout.preferredHeight: 26
                Layout.preferredWidth: 47
                text: OnboardingStrings.done
                onClicked: {
                    if(BackupsModel.renameBackup(mFolder, editTextField.text)) {
                        mName = editTextField.text;
                        content.sourceComponent = confirmContent;
                    }
                }
            }
        }
    }
}
