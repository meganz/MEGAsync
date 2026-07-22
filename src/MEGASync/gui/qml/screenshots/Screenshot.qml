import QtQuick 2.15

import common 1.0

import components.views 1.0
import components.textFields 1.0
import QmlDialog 1.0

QmlDialog {
    id: window

    readonly property int dialogWidth: 640
    readonly property int dialogHeight: 730

    title: ScreenshotStrings.title
    visible: false
    modality: Qt.WindowModal
    color: ColorTheme.pageBackground
    width: dialogWidth
    height: dialogHeight
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width
}
