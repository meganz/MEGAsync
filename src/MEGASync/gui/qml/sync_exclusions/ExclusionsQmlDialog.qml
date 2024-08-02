import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

import common 1.0

import components.images 1.0
import components.texts 1.0 as Texts
import components.buttons 1.0 as Buttons
import components.checkBoxes 1.0
import components.comboBoxes 1.0
import components.textFields 1.0


ExclusionsQmlDialog {
    id: root

    readonly property int dialogMargins: 24
    readonly property int disabledExclusionStatusIndex: 3

    width: 640
    height: 680
    maximumHeight: 680
    maximumWidth: 640
    minimumHeight: 680
    minimumWidth: 640
    title: ExclusionsStrings.addExclusions
    visible: false
    modality: Qt.NonModal
    color: ColorTheme.surface1

    Component.onCompleted: {
        x = Screen.width / 2 - width / 2;
        y = Screen.height / 2 - height / 2;
        visible = true;
    }

    AddRuleDialog {
        id: addExlcusionRule

        visible: false

        onAccepted: {
            if(rulesTable.editIndex === -1){
                syncExclusionsAccess.rulesModel.addNewRule( addExlcusionRule.targetType, addExlcusionRule.valueType,  addExlcusionRule.ruleValue);
            }
            else{
                syncExclusionsAccess.rulesModel.editRule(addExlcusionRule.targetType, addExlcusionRule.valueType,  addExlcusionRule.ruleValue, rulesTable.editIndex);
            }
        }
    }

    Item {
        id: content

        anchors {
            fill: parent
            margins: dialogMargins
        }

        ColumnLayout {
            id: titleDescriptionLayout

            anchors{
                left: parent.left
                right: parent.right
                top: parent.top
            }
            spacing: 4

            Texts.RichText {
                id: manageExclusionTitle

                text: ExclusionsStrings.manageExclusions
                font.pixelSize: Texts.Text.Size.LARGE
                font.weight: Font.DemiBold
            }

            Texts.Text {
                id: manageExclusionDescription

                Layout.preferredWidth: parent.width
                text: ExclusionsStrings.manageExclusionsDescription
                lineHeightMode: Text.FixedHeight
                lineHeight: 18
                font.pixelSize: Texts.Text.Size.NORMAL
            }
        } //ColumnLayout: titleDescriptionLayout

        Texts.RichText {
            id: manageExclusionText

            text: ExclusionsStrings.manageExclusions
            anchors.left: parent.left
            anchors.right: parent.right
            font.pixelSize: Texts.Text.Size.LARGE
            font.weight: Font.DemiBold
        }

        Rectangle{
            id: horizontalLine

            anchors{
                left: parent.left
                right: parent.right
                top: titleDescriptionLayout.bottom
                topMargin: 12
            }
            height: 1
            color: ColorTheme.borderDisabled
        }

        Texts.RichText {
            id: sizeRulesTitle

            anchors{
                left: parent.left
                right: parent.right
                top: horizontalLine.bottom
                topMargin: 12
            }
            text: ExclusionsStrings.sizeRulesTitle
            font.pixelSize: Texts.Text.Size.LARGE
            font.weight: Font.DemiBold
        }

        Texts.Text {
            id: sizeRulesDescription

            anchors{
                left: parent.left
                right: parent.right
                top: sizeRulesTitle.bottom
                topMargin: 12
            }
            text: ExclusionsStrings.sizeRuleDescription
            Layout.preferredWidth: parent.width
            lineHeightMode: Text.FixedHeight
            lineHeight: 18
            font.pixelSize: Texts.Text.Size.NORMAL
        }

        Item{
            id: sizeRuleRow

            anchors{
                left: parent.left
                right: parent.right
                top: sizeRulesDescription.bottom
                topMargin: 12
            }
            height: 36 + sizeRuleCheckbox.sizes.focusBorderWidth * 2
            CheckBox {
                id: sizeRuleCheckbox

                anchors {
                    top: parent.top
                    topMargin: 5 + sizeRuleCheckbox.sizes.focusBorderWidth
                    left: parent.left
                    leftMargin: - sizeRuleCheckbox.sizes.focusBorderWidth
                }
                width: 14 + sizeRuleCheckbox.sizes.focusBorderWidth
                checked: syncExclusionsAccess.sizeExclusionStatus !== disabledExclusionStatusIndex
                onCheckStateChanged: {
                    sizeLimitComboBox.enabled = checked;
                    upperLimitValue.enabled = sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText !== ExclusionsStrings.smallerThan);
                    upperLimitUnit.enabled = sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText !== ExclusionsStrings.smallerThan);
                    andText.enabled = sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText === ExclusionsStrings.outsideOf);
                    lowLimitUnit.enabled = sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText !== ExclusionsStrings.biggerThan);
                    lowLimitValue.enabled = sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText !== ExclusionsStrings.biggerThan);
                    syncExclusionsAccess.sizeExclusionStatus = (checked)? sizeLimitComboBox.currentIndex : disabledExclusionStatusIndex
                }
                focus: true
            }

            Texts.Text {
                id: checkBoxText

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: sizeRuleCheckbox.right
                    leftMargin: 8
                }
                text: ExclusionsStrings.sizeRule
                handlePress: true
                lineHeightMode: Text.FixedHeight
                lineHeight: 20
                font.pixelSize: Texts.Text.Size.MEDIUM
                font.bold: Font.DemiBold
                textMouseArea.cursorShape: Qt.PointingHandCursor
                textMouseArea.onClicked: {
                    sizeRuleCheckbox.checked = (!sizeRuleCheckbox.checked);
                }

            }

            ComboBox {
                id: sizeLimitComboBox

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: checkBoxText.right
                    leftMargin: 12
                }
                implicitWidth: 136
                popupWidth: 210
                currentIndex: syncExclusionsAccess.sizeExclusionStatus === disabledExclusionStatusIndex ? 0 : syncExclusionsAccess.sizeExclusionStatus
                enabled: sizeRuleCheckbox.checked
                model: [
                    ExclusionsStrings.biggerThan,
                    ExclusionsStrings.smallerThan,
                    ExclusionsStrings.outsideOf]
                onActivated:{
                    syncExclusionsAccess.sizeExclusionStatus = currentIndex;
                }
                onCurrentTextChanged: {
                    upperLimitValue.enabled = sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText !== ExclusionsStrings.smallerThan);
                    upperLimitUnit.enabled = sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText !== ExclusionsStrings.smallerThan);
                    andText.enabled = sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText === ExclusionsStrings.outsideOf);
                    lowLimitUnit.enabled = sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText !== ExclusionsStrings.biggerThan);
                    lowLimitValue.enabled = sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText !== ExclusionsStrings.biggerThan);
                }
            }

            TextField {
                id: lowLimitValue

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: sizeLimitComboBox.right
                    leftMargin: 8 - sizeLimitComboBox.sizes.focusBorderWidth - lowLimitValue.sizes.focusBorderWidth
                }
                sizes.iconMargin: 6
                text:  Utilities.formatNumber(syncExclusionsAccess.minimumAllowedSize, 2)
                enabled: sizeRuleCheckbox.checked
                implicitWidth: 48
                colors.border: ColorTheme.borderStrongSelected
                horizontalAlignment: TextInput.AlignRight
                // RegExpValidator to validate numbers from 1 to 999 with up to two decimal places
                validator: RegExpValidator {
                    regExp: RegexExpressions.allow3DigitsOnly
                }

                onTextChanged: {
                    var maxLength =(text.indexOf('.') !== -1)? 4 : 3;
                    if(text.length > maxLength){
                        text = text.substring(0, maxLength);
                    }
                }
                onEditingFinished: {
                    if(text.trim() !== "" && parseFloat(text) > 0){
                        syncExclusionsAccess.minimumAllowedSize = text;
                    } 
                    else {
                        text = 1;
                    }
                }
            }

            ComboBox {
                id: lowLimitUnit

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: lowLimitValue.right
                    leftMargin: 8 - lowLimitUnit.sizes.focusBorderWidth - lowLimitValue.sizes.focusBorderWidth
                }
                implicitWidth: 80
                popupWidth: 107
                currentIndex: syncExclusionsAccess.minimumAllowedUnit
                enabled: sizeRuleCheckbox.checked
                model: [
                    ExclusionsStrings.byteUnit,
                    ExclusionsStrings.kiloByte,
                    ExclusionsStrings.megaByte,
                    ExclusionsStrings.gigaByte]
                onActivated:{
                    syncExclusionsAccess.minimumAllowedUnit = currentIndex;
                }
            }

            Texts.Text {
                id: andText

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: lowLimitUnit.right
                    leftMargin: 8 - lowLimitUnit.sizes.focusBorderWidth
                }
                enabled: sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText === ExclusionsStrings.outsideOf)
                text: ExclusionsStrings.and
                lineHeightMode: Text.FixedHeight
                lineHeight: 20
                font.pixelSize: Texts.Text.Size.MEDIUM
                font.bold: Font.DemiBold
            }

            TextField {
                id: upperLimitValue

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: andText.right
                    leftMargin: 8 - upperLimitValue.sizes.focusBorderWidth
                }
                implicitWidth: 48
                text: Utilities.formatNumber(syncExclusionsAccess.maximumAllowedSize, 2)
                colors.border: ColorTheme.borderStrongSelected
                horizontalAlignment: TextInput.AlignRight
                enabled: sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText === ExclusionsStrings.outsideOf)
                validator: RegExpValidator {
                    regExp: RegexExpressions.allow3DigitsOnly
                }
                sizes.iconMargin: 6
                onTextChanged: {
                    var maxLength =(text.indexOf('.') !== -1)? 4 : 3;
                    if(text.length > maxLength){
                        text = text.substring(0, maxLength);
                    }
                }
                onEditingFinished: {
                    if(text.trim() !== "" && parseFloat(text) > 0){
                        syncExclusionsAccess.maximumAllowedSize = text;
                    } 
                    else {
                        text = 1;
                    }
                }
            }

            ComboBox {
                id: upperLimitUnit

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: upperLimitValue.right
                    leftMargin: 8 - upperLimitValue.sizes.focusBorderWidth - upperLimitUnit.sizes.focusBorderWidth
                }
                implicitWidth: 80
                popupWidth: 107
                currentIndex: syncExclusionsAccess.maximumAllowedUnit
                enabled: sizeRuleCheckbox.checked && (sizeLimitComboBox.currentText === ExclusionsStrings.outsideOf)

                model: [
                    ExclusionsStrings.byteUnit,
                    ExclusionsStrings.kiloByte,
                    ExclusionsStrings.megaByte,
                    ExclusionsStrings.gigaByte]
                onActivated:{
                    syncExclusionsAccess.maximumAllowedUnit = currentIndex;
                }
            }
        }

        Item{
            id: sizeHint

            anchors{
                top: sizeRuleRow.bottom
                topMargin: 12
                left: parent.left
                right: parent.right
            }
            visible: sizeLimitComboBox.currentText === ExclusionsStrings.outsideOf
            SvgImage{
                id: hintIcon

                anchors{
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                }
                source: Images.info
                sourceSize: Qt.size(16, 16)
            }

            Texts.Text {
                id: hintText

                anchors{
                    verticalCenter: parent.verticalCenter
                    left: hintIcon.right
                    leftMargin: 8
                }
                text: ExclusionsStrings.hintText
                lineHeightMode: Text.FixedHeight
                lineHeight: 18
                font.pixelSize: Texts.Text.Size.NORMAL
                verticalAlignment: Text.AlignVCenter
            }
        } // sizeHint

        Texts.RichText {
            id: nameRulesTitle

            text: ExclusionsStrings.excludeByName
            anchors{
                left: parent.left
                right: parent.right
                top: sizeHint.visible? sizeHint.bottom :sizeRuleRow.bottom
                topMargin: 24
            }
            font.pixelSize: Texts.Text.Size.LARGE
            font.weight: Font.DemiBold
        }

        Item{
            id: nameRulesDescriptionItem

            anchors{
                left: parent.left
                right: parent.right
                top: nameRulesTitle.bottom
                topMargin: 8
            }
            implicitHeight: 24

            Texts.Text {
                id: nameRulesDescription

                anchors{
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                }
                text: ExclusionsStrings.nameRulesDescription
                Layout.preferredWidth: parent.width
                lineHeightMode: Text.FixedHeight
                lineHeight: 18
                font.pixelSize: Texts.Text.Size.NORMAL
            }

            Texts.HighlightedText{
                id: folderName

                radius:4
                horizontalPadding: 6
                verticalPadding: 3
                anchors{
                    verticalCenter: parent.verticalCenter
                    left: nameRulesDescription.right
                    leftMargin: 8
                }
                font.bold: Font.DemiBold
                iconSource: Images.refresh
                backgroundColor: ColorTheme.surface2
                text: syncExclusionsAccess.folderName
            }
        } //nameRulesDescriptionItem

        RulesTable{
            id: rulesTable

            anchors{
                left: parent.left
                right: parent.right
                top: nameRulesDescriptionItem.bottom
                topMargin: 8
            }
            onEditRuleClicked: {
                addExlcusionRule.targetType = rulesTable.editRuleTarget
                addExlcusionRule.valueType = rulesTable.editRuleProperty;
                addExlcusionRule.ruleValue = rulesTable.editRuleValue;
                addExlcusionRule.targetEnabled = (rulesTable.editRuleTarget !== 2); // Replace with enum
                addExlcusionRule.valueTypeEnabled = (rulesTable.editRuleTarget !== 2); // Replace with enum
                addExlcusionRule.visible = true;

                if(rulesTable.editIndex === -1) {
                    addExlcusionRule.title = ExclusionsStrings.addExclusion;
                    addExlcusionRule.headTitle = ExclusionsStrings.addExclusion;
                }
                else {
                    addExlcusionRule.title = ExclusionsStrings.editExclusion;
                    addExlcusionRule.headTitle = ExclusionsStrings.editExclusion;
                }
            }
        }

        Buttons.PrimaryButton {
            id: doneButton

            anchors.right: parent.right
            anchors.bottom: parent.bottom
            text: ExclusionsStrings.done
            onClicked: {
                syncExclusionsAccess.rulesModel.applyChanges();
                root.accepted();
                root.close();
            }
        }

        Buttons.OutlineButton {
            id: restoreButton

            anchors.left: parent.left
            anchors.bottom: parent.bottom
            text: ExclusionsStrings.restoreDefaults
            onClicked: {
                syncExclusionsAccess.restoreDefaults();
                root.accepted();
                root.close();
            }
        }
    } // Item: content
} // root

