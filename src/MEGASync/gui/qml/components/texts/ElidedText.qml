import QtQuick 2.15

import components.toolTips 1.0

Text {
    id: elidedText

    property bool showTooltip: true
    property alias tooltip: tooltip
    property int tooltipMaxHorizontal: 0

    maximumLineCount: 1
    elide: Text.ElideMiddle
    horizontalAlignment: Qt.AlignLeft
    verticalAlignment: Qt.AlignVCenter

    MouseArea {
        hoverEnabled: true
        anchors.fill: parent
        enabled: showTooltip

        ToolTip {
            id: tooltip

            visible: showTooltip && elidedText.truncated && parent.containsMouse
            text: elidedText.text
            delay: 500
            timeout: 5000
        }
    }
}
