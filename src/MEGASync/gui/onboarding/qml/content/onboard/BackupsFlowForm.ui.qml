import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Components 1.0 as Custom
import Common 1.0

RowLayout {
    id: root
    height: 520
    width: 776

    BackupInfoStepPanel {
        id: backupInfoStepPanel
        Layout.preferredHeight: root.height
        Layout.preferredWidth: 224

        Connections {
            target: backupContentPanel
            onNextButtonClicked: {
                backupInfoStepPanel.next()
            }
            onPreviousButtonClicked: {
                backupInfoStepPanel.previous()
            }
        }
    }

    Rectangle {
        color: "#FAFAFB"
        Layout.preferredHeight: root.height
        Layout.preferredWidth: root.width - backupInfoStepPanel.width

        ColumnLayout {
            height: parent.height
            width: parent.width

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.leftMargin: 32
                Layout.topMargin: 32
                spacing: 12

                BackupHeader {
                }

                BackupComputerName {
                    Layout.fillWidth: false
                    Layout.preferredWidth: 488
                }
            }

            BackupContentPanel {
                id: backupContentPanel
                Layout.alignment: Qt.AlignBottom
                Layout.bottomMargin: 24
                Layout.leftMargin: 245
            }
        }
    }

    /*
    StackView {
        id: registerStack

        initialItem: loginPage
        anchors {
            left: image.right
            top: root.top
            bottom: root.bottom
            right: root.right
        }
    }*/
} // Rectangle -> root
