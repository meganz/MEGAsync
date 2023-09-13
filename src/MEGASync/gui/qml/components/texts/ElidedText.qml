// System
import QtQuick 2.12

// Local
import Components.Texts 1.0 as MegaTexts
import Components.ToolTips 1.0 as MegaToolTips

MegaTexts.Text {
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

        MegaToolTips.ToolTip {
            id: tooltip

            visible: showTooltip && elidedText.truncated && parent.containsMouse
            text: elidedText.text
            delay: 500
            timeout: 5000
        }

    }
}
