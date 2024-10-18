import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts

QtObject {
    id: root

    property bool adaptableHeight: false

    property real height: 36
    property real maxHeight: 0.0
    property real verticalContentMargin: 4
    property real rightContentMargin: 2
    property real heightContentAdjustment: 12
    property real rightTextAreaMargin: 6
    property real focusBorderRadius: 12
    property real borderRadius: 8
    property real hintTopMargin: 2
    property int focusBorderWidth: Constants.focusBorderWidth
    property int borderWidth: 1
    property int textSize: Texts.Text.Size.NORMAL
    property int hintTextSize: Texts.Text.Size.NORMAL

}
