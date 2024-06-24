import QtQuick 2.15

import components.toolTips 1.0
import components.texts 1.0 as Texts

Texts.Text {
    id: root

    property alias tooltip: tooltip

    property bool showTooltip: true
    property int tooltipMaxHorizontal: 0

    maximumLineCount: 1
    elide: Text.ElideMiddle
    horizontalAlignment: Qt.AlignLeft
    verticalAlignment: Qt.AlignVCenter

    MouseArea {
        id: textMouseArea

        hoverEnabled: true
        anchors.fill: parent
        enabled: showTooltip

        ToolTip {
            id: tooltip

            visible: showTooltip && root.truncated && parent.containsMouse
            text: root.text
            delay: 500
            timeout: 5000
        }
    }
}
