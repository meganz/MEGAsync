import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.15

import Common 1.0
import Components 1.0 as Custom

ColumnLayout {
    id: root

    /*
     * Functions
     */

    function deselect() {
        box.state = box.stateDeselected;
    }

    /*
     * Properties
     */

    property string title: ""
    property string description: ""
    property string imageSource: ""
    property string imageSourceSelected: ""

    /*
     * Signals
     */

    signal selected

    /*
     * Object properties
     */

    width: 488
    height: 96

    /*
     * Child objects
     */

    Button {
        Layout.preferredWidth: parent.width
        Layout.fillWidth: true
        Layout.preferredHeight: parent.height

        background:
            Rectangle {
                id: box

                /*
                 * Properties
                 */
                readonly property string stateSelected: "SELECTED"
                readonly property string stateDeselected: "DESELECTED"

                /*
                 * Object properties
                 */

                border.color: Styles.borderStrong
                border.width: 2
                radius: 8
                color: Styles.surface1

                state: box.stateDeselected

                states: [
                    State {
                        name: box.stateDeselected
                        PropertyChanges { target: box; border.color: Styles.borderStrong }
                        PropertyChanges { target: icon; color: Styles.iconSecondary }
                    },
                    State {
                        name: box.stateSelected
                        PropertyChanges { target: box; border.color: Styles.borderStrongSelected }
                        PropertyChanges { target: icon; color: Styles.iconAccent }
                    }
                ]

                /*
                 * Child objects
                 */

                RowLayout {
                    anchors.fill: parent
                    spacing: 16

                    Custom.SvgImage {
                        id: icon

                        Layout.leftMargin: 24
                        source: imageSource
                    }

                    ColumnLayout {

                        Text {
                            text: title
                            color: Styles.buttonPrimaryHover
                            Layout.preferredHeight: 24
                            font.pixelSize: 16
                            font.weight: Font.Bold
                            font.family: "Inter"
                            font.styleName: "normal"
                            lineHeight: 24
                        }

                        Text {
                            text: description
                            wrapMode: Text.WordWrap
                            lineHeightMode: Text.FixedHeight
                            Layout.preferredWidth: 324
                            color: Styles.toastBackground
                            Layout.preferredHeight: 32
                            font.pixelSize: 10
                            font.weight: Font.Light
                            font.family: "Inter"
                            font.styleName: "normal"
                            lineHeight: 16
                        }
                    }
                }
            } // Rectangle

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                root.selected();

                if(box.state === box.stateDeselected) {
                    box.state = box.stateSelected;
                } else if(box.state === box.stateSelected) {
                    box.state = box.stateDeselected;
                } else {
                    console.error("SyncsInstallationTypeButton: state does not exist -> " + box.state);
                }
            }
        }

    } // Button

} // ColumnLayout

