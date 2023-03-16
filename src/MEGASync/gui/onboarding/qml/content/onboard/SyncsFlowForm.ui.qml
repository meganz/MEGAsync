import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
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

    SyncsInfoStepPanel {
        id: syncsInfoStepPanel

        Layout.preferredHeight: root.height
        Layout.preferredWidth: 224
    }

    Rectangle {
        color: "#FAFAFB"
        Layout.preferredHeight: root.height
        Layout.preferredWidth: root.width - syncsInfoStepPanel.width

        ColumnLayout {
            height: parent.height
            width: parent.width

            SyncsContentPanel {
                id: contentStack
            }

            SyncsFooter {
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
            syncsInfoStepPanel.next();
            contentStack.next();
        }

        onPreviousButtonClicked: {
            syncsInfoStepPanel.previous();
            contentStack.previous();
        }
    }
}
