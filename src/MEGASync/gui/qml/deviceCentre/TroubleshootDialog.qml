import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.buttons 1.0
import components.texts 1.0 as Texts
import components.views 1.0

Window {
    id: root

    readonly property int dialogWidth: 640
    readonly property int dialogHeight: 396
    readonly property int dialogMargin: 24
    readonly property int dialogTopMargin: 52

    property string selectedButtonText

    signal learnMoreClicked
    signal applyPreviousRulesClicked
    signal smartModeSelected
    signal advancedModeSelected

    width: root.dialogWidth
    height: root.dialogHeight
    minimumWidth: root.dialogWidth
    minimumHeight: root.dialogHeight
    maximumWidth: root.dialogWidth
    maximumHeight: root.dialogHeight
    flags: Qt.Dialog
    modality: Qt.WindowModal
    color: ColorTheme.surface1
    title: DeviceCentreStrings.troubleshoot

    onVisibleChanged: {
        if (visible)
        {
            if (deviceCentreAccess.isSmartModeSelected())
            {
                smartOption.checked = true;
                selectedButtonText = smartOption.buttonLabel
            }
            else
            {
                advancedOption.checked = true;
                selectedButtonText = advancedOption.buttonLabel
            }
        }

    }

    ColumnLayout {

        anchors {
            fill: parent
            margins: dialogMargin
        }

        spacing: dialogMargin

        GroupBox {
            id: resolutionModeBox

            title: DeviceCentreStrings.resolutionModeTitle
            Layout.fillWidth: true

            RowLayout {
                id: optionsLayout

                anchors.fill: parent

                ColumnLayout {

                    Layout.fillWidth: true
                    spacing: 4

                    Qml.ButtonGroup {
                        id: resolutionModeGroup
                    }

                    IssueResolutionOption {
                        id: smartOption

                        buttonGroup: resolutionModeGroup
                        buttonLabel: DeviceCentreStrings.smart
                        description: DeviceCentreStrings.smartDescription
                    }

                    IssueResolutionOption {
                        id: advancedOption

                        buttonGroup: resolutionModeGroup
                        buttonLabel: DeviceCentreStrings.advanced
                        description: DeviceCentreStrings.advancedDescription
                    }
                }

                TroubleshootButton {
                    text: DeviceCentreStrings.learnMore
                    onClicked: {
                        root.learnMoreClicked();
                    }
                }
            }
        }

        GroupBox {
            id: exclusionBox

            title: DeviceCentreStrings.exclusionRulesTitle
            Layout.fillWidth: true

            ColumnLayout {

                anchors.fill : parent
                Layout.fillWidth: true

                Texts.SecondaryText {
                    Layout.fillWidth: true
                    text: DeviceCentreStrings.exclusionRulesDescription
                    wrapMode: Text.WordWrap
                }

                TroubleshootButton {
                    text: DeviceCentreStrings.applyPreviousRules
                    onClicked: {
                        root.applyPreviousRulesClicked();
                    }
                }
            }
        }

        // Spacer in QML
        Item {
            Layout.fillHeight: true
        }

        PrimaryButton {
            id: acceptButton

            Layout.alignment: Qt.AlignBottom | Qt.AlignRight

            text: Strings.done

            onClicked: {

                if (smartOption.checked) {
                    if (selectedButtonText !== smartOption.buttonLabel) {
                        root.smartModeSelected();
                    }
                }
                else if (advancedOption.checked) {
                    if (selectedButtonText !== advancedOption.buttonLabel) {
                        root.advancedModeSelected();
                    }
                }
                root.close();
            }
        }
    }
}
