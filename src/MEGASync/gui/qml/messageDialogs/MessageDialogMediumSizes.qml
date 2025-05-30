import QtQuick 2.15

import common 1.0

QtObject {

    readonly property real defaultMinimumWidth: 512.0
    readonly property real defaultMinimumHeight: 208.0
    readonly property real contentMargin: 24.0
    readonly property real numberOfMargins: 2.0
    readonly property real defaultSpacing: 24.0 + Constants.focusAdjustment
    readonly property real iconSize: 100.0
    readonly property real topContentRowSpacing: 24.0
    readonly property real rightContentWidth: 340.0
    readonly property real textColumnSpacing: 8.0
    readonly property real titleTextLineHeight: 24.0
    readonly property real descriptionTextLineHeight: 18.0
    readonly property real bottomButtonsRowSpacing: 0.0

}
