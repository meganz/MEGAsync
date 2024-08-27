import QtQuick 2.0
import QtQuick.Layouts 1.15

import common 1.0
import components.images 1.0
import components.texts 1.0 as Texts

import DeviceOs 1.0
import SyncStatus 1.0

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

        color: colorStyle.surface1;
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

            source: getOSIcon(deviceCenterAccess.getCurrentOS())
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
                color: colorStyle.textPrimary
                wrapMode: Text.NoWrap

                text: "";
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

                    name: DeviceCenterStrings.statusLabel
                    value: DeviceCenterStrings.statusUpToDate
                    icon: Images.statusUpToDate
                }

                DeviceWidgetProperty {
                    id: deviceContains

                    name: DeviceCenterStrings.contains
                    value: DeviceCenterStrings.folderCount(0)
                }

                DeviceWidgetProperty {
                    id: deviceTotalSize

                    name: DeviceCenterStrings.totalSize
                    value: deviceCenterAccess.getSizeString(0)
                }
            }
        }
    }

    onDeviceIdChanged: {
        deviceCenterAccess.retrieveDeviceData(deviceId);
    }

    Connections {
        id: deviceCenterConnection

        target: deviceCenterAccess

        function onDeviceNameReceived(deviceName) {
            deviceTitle.text = deviceName;
        }

        function onDeviceDataUpdated(deviceData) {
            deviceTitle.text = deviceData.name;
            deviceImage.source = getOSIcon(deviceData.os);

            if (deviceData.folderCount > 0)
            {
                deviceStatus.visible = true
                if (deviceData.status === SyncStatus.UP_TO_DATE) {
                    deviceStatus.icon = Images.statusUpToDate
                    deviceStatus.value = DeviceCenterStrings.statusUpToDate
                }
                else if (deviceData.status === SyncStatus.UPDATING) {
                    deviceStatus.icon = Images.statusUpdating
                    deviceStatus.value = DeviceCenterStrings.statusUpdating
                }
                else if (deviceData.status === SyncStatus.PAUSED) {
                    deviceStatus.icon = Images.statusPaused
                    deviceStatus.value = DeviceCenterStrings.statusPaused
                }
                else {
                    deviceStatus.icon = Images.statusStopped
                    deviceStatus.value = DeviceCenterStrings.statusStopped
                }
            }
            else
            {
                deviceStatus.visible = false
            }

            deviceContains.value = DeviceCenterStrings.folderCount(deviceData.folderCount)
            deviceTotalSize.value = deviceCenterAccess.getSizeString(deviceData.totalSize)

        }
    }
}
