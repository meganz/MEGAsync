import QtQuick 2.15

import common 1.0

import PasswordStrengthChecker 1.0

Row {
    id: root

    property int strength: PasswordStrengthChecker.PASSWORD_STRENGTH_VERYWEAK

    readonly property bool isWeak: strength <= PasswordStrengthChecker.PASSWORD_STRENGTH_WEAK
    readonly property bool isModerate: strength === PasswordStrengthChecker.PASSWORD_STRENGTH_MEDIUM

    readonly property int filledSegments: isWeak
                                          ? 1
                                          : (isModerate
                                             ? 2
                                             : (strength === PasswordStrengthChecker.PASSWORD_STRENGTH_GOOD
                                                ? 3
                                                : 4))
    readonly property color fillColor: isWeak
                                       ? ColorTheme.supportError
                                       : (isModerate
                                          ? ColorTheme.supportWarning
                                          : ColorTheme.supportSuccess)

    readonly property int segmentCount: 4

    spacing: 4
    topPadding: 4
    bottomPadding: 4
    // Same disabled treatment as HintText, so the meter dims together with
    // the strength hint below it while the form is disabled.
    opacity: enabled ? 1.0 : 0.2

    Repeater {
        model: root.segmentCount

        Rectangle {
            width: (root.width - (root.segmentCount - 1) * root.spacing) / root.segmentCount
            height: 2
            radius: 4
            color: index < root.filledSegments ? root.fillColor : ColorTheme.surface3
        }
    }
}
