import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.comboBoxes 1.0
import components.textFields 1.0

import ChooseLocalFolder 1.0
import ChooseLocalFile 1.0

Window{
    id: root

    readonly property int dialogWidth: 480
    readonly property int dialogHeight: 370
    readonly property int dialogMargin: 24

    property alias ruleValue: valueTextField.text
    property alias targetType: targetComboBox.currentIndex
    property alias valueType: valueTypeCombo.currentIndex
    property alias targetEnabled: targetComboBox.enabled
    property alias valueTypeEnabled: valueTypeCombo.enabled
    property alias title: root.title
    property alias headTitle: title.text

    function getTextFieldTitle() {
        if (targetComboBox.currentText === ExclusionsStrings.extensions){
            return ExclusionsStrings.filesWithExtension;
        }else if (valueTypeCombo.currentText === ExclusionsStrings.containing){
            if(targetComboBox.currentText === ExclusionsStrings.files){
                return ExclusionsStrings.filesContaining
            }
            else{
                return ExclusionsStrings.foldersContaining
            }
        }
        else if (valueTypeCombo.currentText === ExclusionsStrings.endingWith){
            if(targetComboBox.currentText === ExclusionsStrings.files){
                return ExclusionsStrings.filesEndingWith
            }
            else{
                return ExclusionsStrings.foldersEndingWith
            }
        }
        else if (valueTypeCombo.currentText === ExclusionsStrings.beginningWith){
            if(targetComboBox.currentText === ExclusionsStrings.files){
                return ExclusionsStrings.filesBeginningWith
            }
            else{
                return ExclusionsStrings.foldersBeginningWith
            }
        }
        else if (valueTypeCombo.currentText === ExclusionsStrings.equalTo){
            if(targetComboBox.currentText === ExclusionsStrings.files){
                return ExclusionsStrings.filesEqualTo
            }
            else{
                return ExclusionsStrings.foldersEqualTo
            }
        }
    }

    signal accepted

    onVisibleChanged: {
        if(visible){
            valueTextField.title = getTextFieldTitle();
        }
    }
    width: root.dialogWidth
    height: root.dialogHeight
    minimumWidth: root.dialogWidth
    minimumHeight: root.dialogHeight
    maximumWidth: root.dialogWidth
    maximumHeight: root.dialogHeight
    modality: Qt.WindowModal
    flags: Qt.Dialog
    color: colorStyle.surface1
    title: ExclusionsStrings.addExclusion

    Item {
        id: mainColumn

        anchors {
            fill: parent
            margins: dialogMargin
        }
        Item {
            id: content

            anchors{
                top: parent.top
                right:parent.right
                left:parent.left
            }
            ColumnLayout {
                id: titleDescriptionLayout

                spacing: 4
                anchors{
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                Texts.Text {
                    id: title

                    text: ExclusionsStrings.addExclusion
                    Layout.preferredWidth: parent.width
                    lineHeightMode: Text.FixedHeight
                    lineHeight: 24
                    font {
                        pixelSize: Texts.Text.Size.MEDIUM_LARGE
                        weight: Font.DemiBold
                    }
                }

                Texts.Text {
                    id: body

                    text: ExclusionsStrings.addExclusionDescription
                    Layout.preferredWidth: parent.width
                    lineHeightMode: Text.FixedHeight
                    lineHeight: 18
                    font.pixelSize: Texts.Text.Size.NORMAL
                }
            } // ColumnLayout: titleDescriptionLayout

            Texts.Text {
                id: dropDownsTitle

                anchors{
                    left: parent.left
                    right: parent.right
                    top: titleDescriptionLayout.bottom
                    topMargin: 16
                }
                text: ExclusionsStrings.dropDownsTitle
                lineHeightMode: Text.FixedHeight
                lineHeight: 18
                font {
                    pixelSize: Texts.Text.Size.NORMAL
                    weight: Font.DemiBold
                }
            }

            RowLayout{
                id: comboBoxesLayout

                anchors{
                    left: parent.left
                    right: parent.right
                    leftMargin: -targetComboBox.sizes.focusBorderWidth
                    rightMargin: -targetComboBox.sizes.focusBorderWidth
                    top: dropDownsTitle.bottom
                    topMargin: 4
                }
                spacing: 12  
                ComboBox {
                    id: targetComboBox

                    implicitWidth: 210
                    model: [ExclusionsStrings.files, ExclusionsStrings.folders, ExclusionsStrings.extensions]
                    onActivated:
                    {
                        valueTypeCombo.enabled = currentText != ExclusionsStrings.extensions;
                        valueTextField.title = getTextFieldTitle();
                    }
                }
                ComboBox {
                    id: valueTypeCombo

                    implicitWidth: 210
                    model: [
                        ExclusionsStrings.beginningWith,
                        ExclusionsStrings.containing,
                        ExclusionsStrings.endingWith,
                        ExclusionsStrings.equalTo]
                    onActivated: {
                        valueTextField.title = getTextFieldTitle();
                    }
                }
            }
            ChooseLocalFolder {
                id: folderDialog

                title: ExclusionsStrings.selectFolderTitle
            }
            ChooseLocalFile {
                id: fileDialog

                title: ExclusionsStrings.selectFileTitle
            }
            Connections {
                id: chooseLocalFolderConnection

                target: folderDialog

                function onFolderChoosen(folderPath) {
                    valueTextField.text = folderPath;
                }
            }
            Connections {
                id: chooseLocalFileConnection

                target: fileDialog

                function onFileChoosen(filePath) {
                    valueTextField.text = filePath;
                }
            }
            TextField {
                id: valueTextField

                anchors{
                    left: parent.left
                    right: parent.right
                    leftMargin: -valueTextField.sizes.focusBorderWidth
                    rightMargin: -valueTextField.sizes.focusBorderWidth
                    top: comboBoxesLayout.bottom
                    topMargin: 24
                }
                rightIconVisible: valueTypeCombo.currentText === ExclusionsStrings.equalTo && targetComboBox.currentText !== ExclusionsStrings.extensions
                title: ExclusionsStrings.filesBeginningWith
                placeholderText: ExclusionsStrings.rulePlaceholder
                rightIconSource: Images.plus
                onTextChanged: {
                    root.ruleValue = text;
                }
                rightIconMouseArea.onClicked: {
                    if(targetComboBox.currentText === ExclusionsStrings.files){
                        fileDialog.openRelativeFileSelector(syncExclusionsAccess.folderPath)
                    }
                    else if(targetComboBox.currentText === ExclusionsStrings.folders){
                        folderDialog.openFolderSelector(syncExclusionsAccess.folderPath, false);
                    }
                }
            } // TextField: valueTextField

            Rectangle{
                id: linehorizontalLine

                anchors{
                    left: parent.left
                    right: parent.right
                    leftMargin: -valueTextField.sizes.focusBorderWidth
                    rightMargin: -valueTextField.sizes.focusBorderWidth
                    top: valueTextField.bottom
                    topMargin: 18
                }
                height: 1
                color: colorStyle.borderDisabled
            }
        } // Item: content
        RowLayout {
            id: buttonsLayout

            anchors {
                right: mainColumn.right
                bottom: parent.bottom
                rightMargin: -cancelButton.sizes.focusBorderWidth
                bottomMargin: -cancelButton.sizes.focusBorderWidth
            }
            spacing: 0

            OutlineButton {
                id: cancelButton

                text: ExclusionsStrings.cancel
                onClicked: {
                    root.close();
                }
            }

            PrimaryButton {
                id: acceptButton

                text: ExclusionsStrings.addExclusion
                icons.source: Images.plus
                enabled: valueTextField.text.trim().length !== 0
                icons.position: Icon.Position.LEFT
                onClicked: {
                    root.accepted();
                    root.close();
                }
            }
        } //RowLayou: buttonsLayout
    } // Column: mainColumn
} // Item: root
