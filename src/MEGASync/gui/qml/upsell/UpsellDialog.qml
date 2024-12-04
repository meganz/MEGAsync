import QtQuick 2.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0

import QmlDialog 1.0

QmlDialog {
    id: window

    readonly property int contentMargin: 24

    property int maxWidth: Math.max(544, columnItem.width + 2 * window.contentMargin)
    property int maxHeight: Math.max(495, columnItem.height + 2 * window.contentMargin)

    visible: false
    modality: Qt.NonModal
    width: window.maxWidth
    height: window.maxHeight
    maximumWidth: window.maxWidth
    maximumHeight: window.maxHeight
    minimumWidth: window.maxWidth
    minimumHeight: window.maxHeight

    Column {
        id: columnItem

        anchors {
            left: parent.left
            top: parent.top
            topMargin: window.contentMargin
            leftMargin: window.contentMargin
            rightMargin: window.contentMargin
            bottomMargin: window.contentMargin
        }
        width: content.width
        height: header.height + content.height + columnItem.spacing
        spacing: window.contentMargin + Constants.focusAdjustment

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
