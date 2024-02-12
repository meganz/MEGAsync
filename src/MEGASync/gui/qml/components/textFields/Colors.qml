import QtQuick 2.15

import common 1.0

QtObject {
    id: root

    property color selection: colorStyle.focus
    property color placeholder: colorStyle.textPlaceholder
    property color text: colorStyle.textPrimary
    property color textDisabled: colorStyle.textDisabled
    property color focus: colorStyle.focus
    property color border: colorStyle.borderStrong
    property color borderDisabled: colorStyle.borderDisabled
    property color borderError: colorStyle.textError
    property color borderFocus: colorStyle.borderStrongSelected
    property color background: colorStyle.pageBackground
    property color title: colorStyle.textPrimary
    property color titleDisabled: colorStyle.textDisabled
    property color icon: colorStyle.iconSecondary
    property color iconDisabled: colorStyle.iconDisabled

}
