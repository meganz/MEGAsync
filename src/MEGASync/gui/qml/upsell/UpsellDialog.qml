import QtQuick 2.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0

import QmlDialog 1.0

QmlDialog {
    id: window

    readonly property int contentMargin: 24

    property int totalWidth: Math.max(544, columnItem.width + 2 * window.contentMargin)
    property int totalHeight: columnItem.height + 2 * window.contentMargin

    visible: false
    modality: Qt.NonModal
    width: window.totalWidth
    height: window.totalHeight
    maximumWidth: window.totalWidth
    maximumHeight: window.totalHeight
    minimumWidth: window.totalWidth
    minimumHeight: window.totalHeight

    onTotalHeightChanged: {
        // Force to change height depending on the billed period selected.
        // Maintain this order to resize the window.
        window.minimumHeight = window.totalHeight;
        window.maximumHeight = window.totalHeight;
        window.height = window.totalHeight;
    }

    Component.onCompleted: {
        header.forceActiveFocus();
    }

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
        height: header.height + content.height + window.contentMargin
        spacing: window.contentMargin

        HeaderItem {
            id: header

            width: parent.width
        }

        ContentItem {
            id: content
        }
    }

}
