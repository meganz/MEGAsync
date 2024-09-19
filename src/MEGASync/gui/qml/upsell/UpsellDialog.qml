import QtQuick 2.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0

import QmlDialog 1.0

QmlDialog {
    id: window

    readonly property int contentMargin: 24

    title: Constants.mega
    visible: true
    modality: Qt.NonModal
    width: columnItem.width + 2 * window.contentMargin
    height: columnItem.height + 2 * window.contentMargin
    maximumWidth: columnItem.width + 2 * window.contentMargin
    maximumHeight: columnItem.height + 2 * window.contentMargin
    minimumWidth: columnItem.width + 2 * window.contentMargin
    minimumHeight: columnItem.height + 2 * window.contentMargin

    Column {
        id: columnItem

        readonly property int columnSpacing: 12

        anchors {
            left: parent.left
            top: parent.top
            topMargin: window.contentMargin
            leftMargin: window.contentMargin
            rightMargin: window.contentMargin
            bottomMargin: window.contentMargin - Constants.focusBorderWidth
        }
        width: content.width
        height: header.height + content.height + columnItem.spacing
        spacing: columnItem.columnSpacing

        HeaderItem {
            id: header

            width: parent.width
        }

        ContentItem {
            id: content
        }
    }

    Component.onCompleted: {
        header.forceActiveFocus();
    }

}
