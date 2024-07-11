import QtQuick 2.0

import common 1.0
import components.views 1.0

import DeviceCenterQmlDialog 1.0

DeviceCenterQmlDialog {
    id: window

    readonly property int deviceCenterWidth: 1024
    readonly property int deviceCenterHeight: 720

    title: DeviceCenterStrings.deviceCenterWindowTitle
    visible: true
    modality: Qt.NonModal
    width: deviceCenterWidth
    height: deviceCenterHeight
    maximumHeight: deviceCenterHeight
    maximumWidth: deviceCenterWidth
    minimumHeight: deviceCenterHeight
    minimumWidth: deviceCenterWidth

    LeftSidebar{
        id: leftSidebar

        anchors{
            top: parent.top
            bottom: parent.bottom
            left: parent.left
        }
        width: 256
    }
    Toolbar{
        id: toolbar

        anchors{
            top: parent.top
            left: leftSidebar.right
            right: parent.right
        }
        height: 72
    }
    DevicePage{
        id: content

        anchors{
            top: toolbar.bottom
            bottom: parent.bottom
            left: leftSidebar.right
            right: parent.right
        }
    }
}
