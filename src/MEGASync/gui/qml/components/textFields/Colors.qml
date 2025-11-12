import QtQuick 2.15

import common 1.0

QtObject {
    id: root

    property color selection: ColorTheme.textPrimary
    property color placeholder: ColorTheme.textPlaceholder
    property color text: ColorTheme.textPrimary
    property color textInverse: ColorTheme.textInverse
    property color textDisabled: ColorTheme.textDisabled
    property color focus: ColorTheme.focusColor
    property color border: ColorTheme.borderStrong
    property color borderDisabled: ColorTheme.borderDisabled
    property color borderError: ColorTheme.textError
    property color borderFocus: ColorTheme.borderStrongSelected
    property color background: ColorTheme.pageBackground
    property color title: ColorTheme.textPrimary
    property color titleDisabled: ColorTheme.textDisabled
    property color icon: ColorTheme.iconSecondary
    property color iconDisabled: ColorTheme.iconDisabled

}
