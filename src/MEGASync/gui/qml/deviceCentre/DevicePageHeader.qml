import QtQuick 2.0
import QtQuick.Layouts 1.15

import common 1.0
import components.images 1.0
import components.texts 1.0 as Texts
import components.buttons 1.0 as Buttons

import DeviceOs 1.0
import SyncStatus 1.0
import AppStatsEvents 1.0

Item {
    id:root

    property string deviceId

    function getOSIcon(os) {
        var result = "";
        switch(os) {
        case DeviceOs.LINUX:
            result = Images.pcLinux;
            break;
        case DeviceOs.MAC:
            result = Images.pcMac;
            break;
        case DeviceOs.WINDOWS:
            result = Images.pcWindows;
            break;
        }
        return result;
    }

    Rectangle {
        id: contentItem

        anchors {
            fill: parent;
            leftMargin: 8;
            rightMargin: 8;
            topMargin: 4;
            bottomMargin: 4;
        }

        color: ColorTheme.surface1;
        radius: 8;

        SvgImage {
            id: deviceImage

            anchors {
                left: parent.left;
                leftMargin: 24;
                top: parent.top;
                topMargin: 12;
                bottom: parent.bottom
                bottomMargin: 12;
            }

            source: getOSIcon(deviceCentreAccess.getCurrentOS())
            sourceSize: Qt.size(96, 96)
        }

        Item {
            anchors {
                left: deviceImage.right;
                leftMargin: 24;
                right: parent.right;
                top: parent.top;
                topMargin: 12;
                bottom: parent.bottom
                bottomMargin: 12;
            }

            Texts.RichText {
                id: deviceTitle

                anchors {
                    left: parent.left
                    bottom: parent.verticalCenter
                    bottomMargin: 5
                }

                font {
                    pixelSize: Texts.Text.Size.MEDIUM_LARGE
                    weight: Font.DemiBold
                }
                color: ColorTheme.textPrimary
                wrapMode: Text.NoWrap

                text: "";
            }

            Buttons.IconButton {
                id: menuButton

                anchors {
                    left: deviceTitle.right
                    verticalCenter: deviceTitle.verticalCenter
                    leftMargin: Constants.focusAdjustment + 8
                }

                icons.source: Images.threeDots
                visible: true
                sizes.iconWidth: 16

                onClicked: {
                    renameDialog.initialName =  deviceTitle.text;
                    renameDialog.deviceName =  deviceTitle.text;
                    renameDialog.visible = true;
                    proxyStatsEventHandlerAccess.sendTrackedEvent(AppStatsEvents.DEVICE_CENTRE_RENAME_ICON_CLICKED);
                }

                RenameDeviceDialog{
                    id: renameDialog

                    visible: false
                    onAccepted: {
                        proxyStatsEventHandlerAccess.sendTrackedEvent(AppStatsEvents.DEVICE_CENTRE_RENAME_ICON_CLICKED);
                        deviceCentreAccess.renameCurrentDevice(renameDialog.deviceName);
                    }
                }
            }

            RowLayout {

                spacing: 150

                anchors {
                    left: parent.left
                    top: parent.verticalCenter
                    topMargin: 5
                }

                DeviceWidgetProperty {
                    id: deviceStatus

                    name: DeviceCentreStrings.statusLabel
                    value: DeviceCentreStrings.statusUpToDate
                    icon: Images.statusUpToDate
                }

                DeviceWidgetProperty {
                    id: deviceContains

                    name: DeviceCentreStrings.contains
                    value: DeviceCentreStrings.folderCount(0)
                }

                DeviceWidgetProperty {
                    id: deviceTotalSize

                    name: DeviceCentreStrings.totalSize
                    value: deviceCentreAccess.getSizeString(0)
                }
            }
        }
    }

    onDeviceIdChanged: {
        deviceCentreAccess.retrieveDeviceData(deviceId);
    }

    Connections {
        id: deviceCentreConnection

        target: deviceCentreAccess

        function onDeviceNameReceived(deviceName) {
            deviceTitle.text = deviceName;
        }

        function onDeviceDataUpdated() {
            deviceTitle.text = deviceCentreAccess.deviceData.name;
            deviceImage.source = getOSIcon(deviceCentreAccess.deviceData.os);

            if (deviceCentreAccess.deviceData.folderCount > 0)
            {
                deviceStatus.visible = true
                if (deviceCentreAccess.deviceData.status === SyncStatus.UP_TO_DATE) {
                    deviceStatus.icon = Images.statusUpToDate
                    deviceStatus.value = DeviceCentreStrings.statusUpToDate
                }
                else if (deviceCentreAccess.deviceData.status === SyncStatus.UPDATING) {
                    deviceStatus.icon = Images.statusUpdating
                    deviceStatus.value = DeviceCentreStrings.statusUpdating
                }
                else if (deviceCentreAccess.deviceData.status === SyncStatus.PAUSED) {
                    deviceStatus.icon = Images.statusPaused
                    deviceStatus.value = DeviceCentreStrings.statusPaused
                }
                else {
                    deviceStatus.icon = Images.statusStopped
                    deviceStatus.value = DeviceCentreStrings.statusStopped
                }
            }
            else
            {
                deviceStatus.visible = false
            }

            deviceContains.value = DeviceCentreStrings.folderCount(deviceCentreAccess.deviceData.folderCount)
            deviceTotalSize.value = deviceCentreAccess.getSizeString(deviceCentreAccess.deviceData.totalSize)

        }
    }
}
