import QtQuick 2.15
import QtQuick.Window 2.15

import common 1.0

import components.textFields 1.0
import components.buttons 1.0 as Buttons

import AppStatsEvents 1.0

Window {
    id: root

    readonly property int dialogWidth: 368
    readonly property int dialogHeight: 144
    readonly property int dialogMargin: 24
    property alias deviceName: deviceNameTextFieldComp.text
    property string initialName: ""

    signal accepted;

    function closeRoutine() {
        showErrorRoutine(DeviceName.Error.NONE);
        root.close();
        root.height = root.dialogHeight;
    }

    function showErrorRoutine(error) {
        root.height = error !== DeviceName.Error.NONE? 197 : root.height;
        DeviceName.showErrorRoutine(deviceNameTextFieldComp, error)
    }

    title: DeviceCentreStrings.renameDevice
    width: root.dialogWidth
    height: root.dialogHeight
    minimumWidth: root.dialogWidth
    minimumHeight: root.dialogHeight
    flags: Qt.Dialog
    modality: Qt.WindowModal
    color: ColorTheme.surface1

    Item {
        id: content

        anchors.fill: parent
        anchors.margins: root.dialogMargin

        TextField {
            id: deviceNameTextFieldComp

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: Constants.focusAdjustment
            focus: true
            hint.icon: Images.alertTriangle
            sizes: Sizes {}
            leftIconSource: Images.monitor

            onTextChanged: {
                if(error) {
                    showErrorRoutine(DeviceName.Error.NONE)
                }
            }
        }

        Buttons.PrimaryButton {
            id: renameButton

            anchors{
                right: parent.right
                bottom: parent.bottom
                margins: Constants.focusAdjustment
            }
            text: DeviceCentreStrings.rename
            icons {
                source: Images.edit
                position: Buttons.Icon.Position.LEFT
            }

            onClicked: {
                let inputText = deviceNameTextFieldComp.text.trim();
                if(initialName === inputText){
                    root.closeRoutine();
                    return;
                }
                let error = DeviceName.getErrorCode(deviceCentreAccess, inputText);
                showErrorRoutine(error);
                if (error !== DeviceName.Error.NONE)
                {
                    return;
                }
                proxyStatsEventHandlerAccess.sendTrackedEvent(AppStatsEvents.RENAME_DIALOG_RENAME_BUTTON_CLICKED);
                root.accepted();
                root.closeRoutine();
            }
        }

        Buttons.OutlineButton {
            id: cancelButton

            anchors{
                right: renameButton.left
                rightMargin: 8 + Constants.focusAdjustment
                bottom: parent.bottom
                margins: Constants.focusAdjustment
            }
            text: Strings.cancel

            onClicked: {
                proxyStatsEventHandlerAccess.sendTrackedEvent(AppStatsEvents.RENAME_DIALOG_CANCEL_BUTTON_CLICKED);
                root.closeRoutine();
            }
        }
    }
}
