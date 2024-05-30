import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import common 1.0

import components.checkBoxes 1.0
import components.texts 1.0 as Texts
import components.images 1.0
import components.buttons 1.0 as Buttons
import components.dialogs 1.0
import WildCardEnum 1.0

Rectangle {
    id: root

    readonly property int tableRadius: 6
    readonly property int filesTargetIndex: 0
    readonly property int extensionsTargetIndex: 2

    property int  editRuleTarget: 0
    property int  editRuleProperty: 0
    property string  editRuleValue: ""
    property int editIndex: -1
    property int removedIndex: -1


    function getConfirmationMessage(target, wildcard, value) {
        if (target === root.extensionsTargetIndex){
            return ExclusionsStrings.removeExtension.arg(value);
        }else if (wildcard === WildCard.CONTAINS){
            if(target === filesTargetIndex){
                return ExclusionsStrings.removeFilesContaining.arg(value)
            }
            else{
                return ExclusionsStrings.removeFoldersContaining.arg(value)
            }
        }
        else if (wildcard === WildCard.ENDSWITH){
            if(target === filesTargetIndex){
                return ExclusionsStrings.removeFilesEnding.arg(value)
            }
            else{
                return ExclusionsStrings.removeFoldersEnding.arg(value)
            }
        }
        else if (wildcard === WildCard.STARTSWITH){
            if(target === filesTargetIndex){
                return ExclusionsStrings.removeFilesBeginning.arg(value)
            }
            else{
                return ExclusionsStrings.removeFoldersBeginning.arg(value)
            }
        }
        else if (wildcard === WildCard.EQUAL){
            if(target === filesTargetIndex){
                return ExclusionsStrings.removeFilesEqual.arg(value)
            }
            else{
                return ExclusionsStrings.removeFoldersEqual.arg(value)
            }
        }
    }

    signal editRuleClicked

    Layout.preferredWidth: parent.width
    Layout.preferredHeight: height
    height: 242
    width: parent.width

    color: colorStyle.pageBackground


    ConfirmCloseDialog {
        id: removeRuleConfirmation

        titleText: ExclusionsStrings.removeConfirmationTitle
        cancelButtonText: ExclusionsStrings.cancel
        acceptButtonText: ExclusionsStrings.removeButon
        acceptButtonColors.background: colorStyle.buttonError
        acceptButtonColors.hover: colorStyle.buttonErrorHover
        acceptButtonColors.pressed: colorStyle.buttonErrorPressed
        dontAskAgainVisible: true
        dontAskAgainText:  ExclusionsStrings.askAgainText
        dontAskAgain: true
        enableBusyIndicator: false
        visible: false
        onAccepted: {
            if(removedIndex !== -1){
                tableView.model.removeRow(removedIndex);
                removeRuleConfirmation.close();
                removedIndex = -1;
            }
            syncExclusionsAccess.askOnExclusionRemove = !dontAskAgain
        }
    }

    Rectangle {
        id: headerRect

        anchors
        {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        implicitHeight: 24 + tableRadius
        color: colorStyle.surface2
        z: 1
        radius: tableRadius
        border{
            color: colorStyle.borderSubtle
            width: 1
        }
        Item{
            id: typeColumnHeaderItem

            anchors.left: parent.left
            anchors.top: parent.top
            height: parent.height
            width: 190

            CheckBox {
                id: enableAll

                property bool fromModel: false

                anchors{
                    top: parent.top
                    topMargin: 4 - enableAll.sizes.focusBorderWidth
                    left: parent.left
                    leftMargin: 12 - enableAll.sizes.focusBorderWidth
                }
                implicitWidth: 16

                tristate: true
                checkState: tableView.model.enabledRulesStatus
                onCheckStateChanged: {
                    if(!enableAll.fromModel){
                        tableView.model.enabledRulesStatus = checkState;
                    }
                    enableAll.fromModel = false;
                }

                Component.onCompleted: {
                    enableAll.checkState = tableView.model.enabledRulesStatus;
                }

                Connections {
                    target:  tableView.model

                    function onEnabledRulesStatusChanged() {
                        enableAll.fromModel = true;
                        enableAll.checkState = tableView.model.enabledRulesStatus;
                    }
                }
            }

            Texts.Text {
                id: typeHeaderText

                anchors{
                    top: parent.top
                    topMargin: 6
                    left: enableAll.right
                    leftMargin: 12
                }
                text: ExclusionsStrings.typeHeader
                Layout.preferredWidth: parent.width
                lineHeightMode: Text.FixedHeight
                lineHeight: 16
                font.bold: Font.DemiBold
                font.pixelSize: Texts.Text.Size.SMALL
            }
        } // typeColumnHeaderItem

        Item{
            id:  propertyColumnHeaderItem

            height: parent.height
            width: 180
            anchors{
                top: parent.top
                left: typeColumnHeaderItem.right
            }
            Texts.Text {
                id: propertyHeaderText

                anchors{
                    top: parent.top
                    topMargin: 6
                    left: parent.left
                }
                text: ExclusionsStrings.propertyHeader
                Layout.preferredWidth: parent.width
                lineHeightMode: Text.FixedHeight
                lineHeight: 16
                font.bold: Font.DemiBold
                font.pixelSize: Texts.Text.Size.SMALL
            }
        } // propertyColumnHeaderItem
        Item{
            id:  valueColumnHeaderItem

            anchors{
                top: parent.top
                left: propertyColumnHeaderItem.right
            }
            height: parent.height
            width: 172

            Texts.Text {
                id: valueHeaderText

                anchors{
                    top: parent.top
                    topMargin: 6
                    left: parent.left
                    leftMargin: 2
                }
                text: ExclusionsStrings.valueHeader
                Layout.preferredWidth: parent.width
                lineHeightMode: Text.FixedHeight
                lineHeight: 16
                font.bold: Font.DemiBold
                font.pixelSize: Texts.Text.Size.SMALL
            }
        } // valueColumnHeaderItem
    } // headerRect

    Rectangle{
        id: tableRect

        anchors{
            left: parent.left
            right: parent.right
            top: headerRect.bottom
            topMargin: -1 * tableRadius
            bottom: footerRect.top
            bottomMargin: -1 * tableRadius
        }

        border{
            color: colorStyle.borderSubtle
            width: 1
        }
        z: 2
        TableView {
            id: tableView

            anchors{
                fill: parent
            }
            rowDelegate: Rectangle {
                height: 32
                width: parent.width
                color: styleData.row % 2 === 0 ? colorStyle.pageBackground : colorStyle.surface1// Alternating colors
            }
            headerDelegate: Rectangle {
                visible: false
            }
            model: syncExclusionsAccess.rulesModel
            focus: true
            TableViewColumn {
                id: typeColumn

                role: "type"
                title: "type"
                width: 190

                delegate:
                    Item{
                    id: firstColumnItem

                    anchors.fill: parent
                    CheckBox {
                        id: ruleCheck

                        implicitWidth: 16
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 12 - ruleCheck.sizes.focusBorderWidth
                        manageChecked: true
                        checked: (model)? !model.commented : false

                        mouseArea.onClicked: {
                            model.commented = !model.commented;
                        }
                        Keys.onPressed: {
                            if (event.key === Qt.Key_Return
                                    || event.key === Qt.Key_Enter
                                    || event.key === Qt.Key_Space) {
                                model.commented = !model.commented;
                            }
                        }
                    }

                    Texts.HighlightedText {
                        id: typeText

                        anchors{
                            left: ruleCheck.right
                            leftMargin: 12
                            verticalCenter: parent.verticalCenter
                        }
                        backgroundColor: colorStyle.indicatorBackground
                        radius: 4
                        iconSource: model? Images.imagesExclusionsPath + model.iconName + '.svg': ""
                        iconSize:  Qt.size(16, 16)
                        text: model? model.type : ""
                    }

                }
            }

            TableViewColumn {
                id: propertyColumn

                role: "property";
                title: "property";
                width: 172
                delegate:
                    Item{
                    id: propertyColumnItem

                    Texts.HighlightedText{
                        id: propertyText

                        anchors{
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                        backgroundColor: colorStyle.notificationInfo
                        radius: 4
                        text: model? model.property : ""
                    }
                }
            }

            TableViewColumn {
                id: valueColumn

                role: "value";
                title: "value";
                width: 150
                delegate: Item{
                    id: valueColumnItem

                    Texts.ElidedText {
                        id: valueText

                        anchors{
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                            leftMargin: 8
                        }
                        width: valueColumn.width - 5
                        font.pixelSize: Texts.Text.Size.SMALL
                        text: model ? model.value: ""
                        color: colorStyle.textPrimary
                        elide: Text.ElideMiddle
                        wrapMode: Text.NoWrap
                    }
                }
            }

            TableViewColumn {
                id: buttonsColumn

                width:64

                delegate:Item {
                    id: buttonsColumnItem

                    Buttons.SecondaryButton {
                        id: editButton

                        anchors{
                            left: parent.left
                        }
                        icons.source: Images.editRule
                        sizes: Buttons.SmallSizes {
                            borderLess: true
                        }
                        colors.background: "transparent"
                        onClicked: {
                            editRuleProperty = model.wildcard;
                            editIndex = model.index;
                            editRuleTarget = model.targetTypeIndex;
                            editRuleValue = model.value;
                            editRuleClicked();
                        }
                    }

                    Buttons.SecondaryButton {
                        id: removeButton

                        anchors{
                            left: editButton.right
                            right: parent.right
                        }
                        sizes: Buttons.SmallSizes {
                            borderLess: true
                        }
                        icons.source: Images.xSquare
                        colors.background: "transparent"
                        colors.borderSelected: "transparent"
                        onClicked: {
                            if(!syncExclusionsAccess.askOnExclusionRemove){
                                tableView.model.removeRow(model.index);
                                return;
                            }
                            removedIndex = model.index;
                            removeRuleConfirmation.bodyText = getConfirmationMessage(model.targetTypeIndex, model.wildcard, model.value)
                            removeRuleConfirmation.visible = true;
                        }
                    }
                }
            } // buttonsColumn
        } // tableView
    } // tableRect
    Connections {
        id: rulesModelConnection

        target: syncExclusionsAccess.rulesModel

        function onNewRuleAdded(addedRuleIndex) {
            tableView.positionViewAtRow(addedRuleIndex, TableView.AlignCenter);
        }
    }
    Rectangle{
        id: footerRect

        anchors{
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: addRuleButton.height + tableRadius + 2 * 8

        radius: tableRadius
        color: colorStyle.surface1
        border{
            color: colorStyle.borderSubtle
            width: 1
        }

        Buttons.PrimaryButton {
            id: addRuleButton

            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            sizes: Buttons.SmallSizes{}
            text: ExclusionsStrings.addExclusion
            icons.source: Images.plus
            icons.position:  Buttons.Icon.Position.LEFT
            onClicked: {
                editRuleProperty = 0;
                editIndex = -1;
                editRuleTarget = 0;
                editRuleValue = "";
                editRuleClicked();
            }
        }
    } // footerRect
}
