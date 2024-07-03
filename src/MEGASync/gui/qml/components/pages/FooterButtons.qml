import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.buttons 1.0

RowLayout {
    id: root

    property alias leftPrimary: leftPrimaryButton // 1st
    property alias leftSecondary: leftSecondaryButton // 2nd

    property alias rightTertiary: rightTertiaryButton // 3rd
    property alias rightSecondary: rightSecondaryButton // 2nd
    property alias rightPrimary: rightPrimaryButton // 1st

    anchors {
        bottom: parent.bottom
        right: parent.right
        left: parent.left
        leftMargin: Constants.focusAdjustment
        bottomMargin: Constants.focusAdjustment
        rightMargin: Constants.focusAdjustment
    }

    //       left        |             right
    // +-----+ +-----+        +-----+ +-----+ +-----+
    // | 1st | | 2nd |   |    | 3rd | | 2nd | | 1st |
    // +-----+ +-----+        +-----+ +-----+ +-----+
    //                   |
    // Type:
    //    Out     Sec    |      Out     Out    Prim
    //
    // Out = Outline ; Sec = Secondary ; Prim = Primary
    //
    // Default visibility:
    //   YES      NO     |      NO      YES     YES
    //
    // Default onClicked:
    //   YES      NO     |      YES      NO     NO

    RowLayout {
        id: leftButtonsLayout

        spacing: 0
        Layout.alignment: Qt.AlignLeft

        OutlineButton {
            id: leftPrimaryButton
        }

        SecondaryButton {
            id: leftSecondaryButton

            visible: false
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
