import QtQuick 2.12
import QtQuick.Layouts 1.12

import Common 1.0
import Components 1.0 as Custom
import Onboard.Syncs_types 1.0

ColumnLayout {
    id: mainLayout

    width: parent.width
    spacing: 24
    onVisibleChanged: {
        if(visible) {
            footerLayout.nextButton.iconSource = "../../../images/Onboarding/cloud.svg";
            footerLayout.nextButton.sourceSize = Qt.size(25, 25);
            footerLayout.nextButton.text = qsTr("Backup");
        } else {
            footerLayout.nextButton.iconSource = "../../../images/Onboarding/arrow_right.svg";
            footerLayout.nextButton.sourceSize = Qt.size(10, 10);
            footerLayout.nextButton.text = qsTr("Next");
        }
    }

    Rectangle {
        Layout.preferredWidth: mainLayout.width
        Layout.preferredHeight: 176
        color: Styles.pageBackground
        border.color: Styles.borderStrong
        border.width: 2
        radius: 8
    }

    ColumnLayout {

        spacing: 4

        Text {
            text: qsTr("Backup to:")
            font.pixelSize: 12
            font.weight: Font.DemiBold
            font.family: "Inter"
            font.styleName: "normal"
        }

        Rectangle {
            Layout.preferredWidth: mainLayout.width
            Layout.preferredHeight: 36
            color: Styles.pageBackground
            border.color: Styles.borderStrong
            border.width: 2
            radius: 8

            RowLayout {
                id: backupToInfo

                spacing: 6
                anchors.verticalCenter: parent.verticalCenter

                Custom.SvgImage {
                    Layout.leftMargin: 10
                    source: "../../../../../../images/Onboarding/cloud.svg"
                    sourceSize: Qt.size(25, 25)
                    color: Styles.iconAccent
                }

                Text {
                    text: qsTr("/Backups")
                    font.family: "Inter"
                    font.styleName: "normal"
                    font.weight: Font.Thin
                    font.pixelSize: 14
                }
            }
        }

    } // ColumnLayout

} // ColumnLayout -> mainLayout
