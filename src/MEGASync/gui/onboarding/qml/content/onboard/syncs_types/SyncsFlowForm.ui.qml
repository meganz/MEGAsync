import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Onboard.Syncs_types.Left_panel 1.0
import Components 1.0 as Custom
import Common 1.0

RowLayout {
    id: root

    /*
     * Object properties
     */

    height: 520
    width: 776

    /*
     * Child objects
     */

    StepPanel {
        id: syncsStepPanel

        Layout.preferredHeight: root.height
        Layout.preferredWidth: 224
    }

    Rectangle {
        color: "#FAFAFB"
        Layout.preferredHeight: root.height
        Layout.preferredWidth: root.width - syncsStepPanel.width

        ColumnLayout {
            height: parent.height
            width: parent.width

            ContentPanel {
                id: contentStack
            }

            Footer {
                id: syncsFooter

                Layout.alignment: Qt.AlignBottom
                Layout.bottomMargin: 24
                Layout.leftMargin: 245
            }

        }

    }

    Connections {
        target: syncsFooter

        onNextButtonClicked: {
            syncsStepPanel.next();
            contentStack.next();
        }

        onPreviousButtonClicked: {
            syncsStepPanel.previous();
            contentStack.previous();
        }
    }
}
