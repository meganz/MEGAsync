import QtQuick 2.0
import QtQuick.Window 2.15

import common 1.0
import components.views 1.0

import QmlDialog 1.0

QmlDialog {
    id: window

    title: DeviceCentreStrings.windowTitle
    visible: true
    modality: Qt.NonModal
    width: 1024
    height: 720
    maximumHeight: 720
    maximumWidth: 1024
    minimumHeight: 720
    minimumWidth: 1024
    color: ColorTheme.pageBackground

    Component.onCompleted: {
        x: Math.round((Screen.desktopAvailableWidth - width) / 2)
        y: Math.round((Screen.desktopAvailableHeight - height) / 2)
    }

    LeftSidebar {
        id: leftSidebar

        anchors{
            top: parent.top
            bottom: parent.bottom
            left: parent.left
        }
        width: 256
    }

    Toolbar {
        id: toolbar

        anchors{
            top: parent.top
            left: leftSidebar.right
            right: parent.right
        }
        height: 72
    }

    DevicePage {
        id: content

        anchors{
            top: toolbar.bottom
            bottom: parent.bottom
            left: leftSidebar.right
            right: parent.right
        }
    }
}
