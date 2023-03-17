import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import Common 1.0
import Components 1.0 as Custom

Rectangle {

    /*
     * Object properties
     */

    width: 488
    height: 48
    color: Styles.pageBackground
    border.color: Styles.borderStrong
    border.width: 2
    radius: 8

    /*
     * Child objects
     */

    RowLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 24
            spacing: 8

            Custom.SvgImage {
                source: "../../../../images/Onboarding/shield.svg"
            }

            Text {
                // TODO: change by real value
                text: qsTr("Free")
                Layout.alignment: Qt.AlignLeft
                font.family: "Inter"
                font.styleName: "normal"
                font.weight: Font.DemiBold
                font.pixelSize: 14
                font.underline: true

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        console.debug("TODO: show plan info");
                    }
                }
            }

        } // RowLayout -> left

        RowLayout {
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: 24

            Text {
                text: qsTr("Storage space")
                font.family: "Inter"
                font.styleName: "normal"
                font.weight: Font.DemiBold
                font.pixelSize: 12
            }

            Text {
                // TODO: change by real value
                text: "979.39 KB"
                font.family: "Inter"
                font.styleName: "normal"
                font.weight: Font.DemiBold
                font.pixelSize: 12
            }

            Text {
                // TODO: change by real value
                text: "/ 20 GB"
                font.family: "Inter"
                font.styleName: "normal"
                font.weight: Font.ExtraLight
                font.pixelSize: 12
            }

        } // RowLayout -> right

    } // RowLayout -> main

} // Rectangle
