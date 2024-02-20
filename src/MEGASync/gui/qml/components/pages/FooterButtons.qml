import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.buttons 1.0

RowLayout {
    id: root

    property alias leftIcon: leftIconButton
    property alias leftSecondary: leftSecondaryButton
    property alias rightTertiary: rightTertiaryButton
    property alias rightSecondary: rightSecondaryButton
    property alias rightPrimary: rightPrimaryButton

    anchors {
        bottom: parent.bottom
        right: parent.right
        left: parent.left
        leftMargin: -leftSecondary.sizes.focusBorderWidth
        bottomMargin: -leftSecondary.sizes.focusBorderWidth
        rightMargin: -rightPrimary.sizes.focusBorderWidth
    }

    //        left               |                    right
    // +------+ +-----------+        +----------+ +-----------+ +---------+
    // | icon | | secondary |        | tertiary | | secundary | | primary |
    // +------+ +-----------+        +----------+ +-----------+ +---------+
    //
    // Default visibility:
    //    NO         YES                  NO           YES          YES

    RowLayout {
        id: leftButtonsLayout

        spacing: 0
        Layout.alignment: Qt.AlignLeft

        IconButton {
            id: leftIconButton

            icons.source: Images.helpCircle
            visible: false
        }

        OutlineButton {
            id: leftSecondaryButton

            text: Strings.skip
            onClicked: {
                window.close();
            }
        }
    }

    RowLayout {
        id: rightButtonsLayout

        spacing: 0
        Layout.alignment: Qt.AlignRight

        OutlineButton {
            id: rightTertiaryButton

            text: Strings.cancel
            visible: false
            onClicked: {
                window.close();
            }
        }

        OutlineButton {
            id: rightSecondaryButton

            text: Strings.previous
        }

        PrimaryButton {
            id: rightPrimaryButton

            text: Strings.next
            icons.source: Images.arrowRight
        }
    }
}
