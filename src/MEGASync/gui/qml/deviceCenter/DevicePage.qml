import QtQuick 2.0
import components.texts 1.0 as Texts

Item {
    id:root

    readonly property int verticalMargins: 24
    readonly property int headerHeight: 128

    DevicePageHeader{
        id: header

        anchors{
            top: parent.top
            left:parent.left
            right: parent.right
            topMargin: root.verticalMargins
        }
        height: root.headerHeight
    } //header
    DevicePageContent{
        id: content

        anchors{
            top: header.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
    }// content

    Component.onCompleted: {
        var deviceId = deviceCenterAccess.getThisDeviceId();
        header.deviceId = deviceId;
    }
} //root
