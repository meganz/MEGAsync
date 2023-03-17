import QtQuick 2.12
import QtQuick.Layouts 1.12

import Common 1.0
import Components 1.0 as Custom
import Onboard.Syncs_types 1.0

ColumnLayout {

    width: parent.width

    /*
     * Child objects
     */

    InfoAccount {
    }

    Rectangle {
        Layout.preferredWidth: 488
        Layout.preferredHeight: 186
        Layout.topMargin: 8
        color: Styles.pageBackground
        border.color: Styles.borderStrong
        border.width: 2
        radius: 8
    }

    Rectangle {
        Layout.topMargin: 12

        RowLayout {
            id: addFolders
            spacing: 9

            Custom.SvgImage {
                Layout.leftMargin: 19
                source: "../../../../../../images/Onboarding/plus-circle.svg"
                color: Styles.buttonPrimary
            }

            Text {
                text: qsTr("Add folders")
                font.family: "Inter"
                font.styleName: "normal"
                font.weight: Font.DemiBold
                font.pixelSize: 12
                font.underline: true
            }
        }

        MouseArea {
            anchors.fill: addFolders
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                console.debug("TODO: Add folders");
            }
        }

    }

    Rectangle {
        Layout.preferredWidth: 488
        Layout.preferredHeight: 186
        Layout.topMargin: 8
        color: Styles.pageBackground
        border.color: Styles.borderStrong
        border.width: 2
        radius: 8
    }

    Rectangle {
        Layout.topMargin: 12

        RowLayout {
            id: addFolders
            spacing: 9

            Custom.SvgImage {
                Layout.leftMargin: 19
                source: "../../../../../../images/Onboarding/plus-circle.svg"
                color: Styles.buttonPrimary
            }

            Text {
                text: qsTr("Add folders")
                font.family: "Inter"
                font.styleName: "normal"
                font.weight: Font.DemiBold
                font.pixelSize: 12
                font.underline: true
            }
        }

        MouseArea {
            anchors.fill: addFolders
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                console.debug("TODO: Add folders");
            }
        }
    }

}
