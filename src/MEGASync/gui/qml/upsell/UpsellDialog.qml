import QtQuick 2.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0

import UpsellComponents 1.0

UpsellQmlDialog {
    id: window

    readonly property int contentMargin: 24
    readonly property int numberOfMargins: 2
    readonly property int defaultMinimumWidth: 544

    property int totalWidth: Math.max(window.defaultMinimumWidth,
                                      columnItem.width + window.numberOfMargins * window.contentMargin)
    property int totalHeight: columnItem.height + window.numberOfMargins * window.contentMargin

    visible: false
    modality: Qt.NonModal
    width: window.totalWidth
    height: window.totalHeight
    maximumWidth: window.totalWidth
    maximumHeight: window.totalHeight
    minimumWidth: window.totalWidth
    minimumHeight: window.totalHeight

    Component.onCompleted: {
        header.forceActiveFocus();
    }

    Column {
        id: columnItem

        anchors {
            left: parent.left
            top: parent.top
            margins: window.contentMargin
        }

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
