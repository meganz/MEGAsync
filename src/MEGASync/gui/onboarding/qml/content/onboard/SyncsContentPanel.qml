import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Components 1.0 as Custom

Item {

    /*
     * Functions
     */

    function next() {
        if(configurationStack.currentItemStack === configurationStack.itemStack.length-1) {
            console.error("ButtonContentPanel: configurationStack.currentItemStack out of bounds on next");
            return;
        }

        configurationStack.replace(configurationStack.itemStack[++configurationStack.currentItemStack],
                                  StackView.Immediate);
    }

    function previous() {
        if(configurationStack.currentItemStack === 0) {
            console.error("ButtonContentPanel: configurationStack.currentItemStack out of bounds on previous");
            return;
        }

        configurationStack.replace(configurationStack.itemStack[--configurationStack.currentItemStack],
                                   StackView.Immediate);
    }

    /*
     * Object properties
     */

    width: parent.width

    /*
     * Child objects
     */

    StackView {
        id: configurationStack

        property int currentItemStack: 0
        property var itemStack: [
            computerNamePage,
            installationTypePage
        ]

        width: parent.width
        initialItem: computerNamePage

        Component {
            id: computerNamePage

            ColumnLayout {

                spacing: 12

                SyncsHeader {
                    title: qsTr("Set up MEGA")
                    description: qsTr("You can assign the name for personal use or workgroup membership of this computer.")
                    Layout.fillWidth: false
                    Layout.preferredWidth: 488
                    Layout.topMargin: 32
                    Layout.leftMargin: 32
                }

                SyncsComputerName {
                    Layout.fillWidth: false
                    Layout.preferredWidth: 488
                    Layout.leftMargin: 32
                }

            }

        } // Component -> computerNamePage

        Component {
            id: installationTypePage

            ColumnLayout {

                spacing: 12

                SyncsHeader {
                    title: qsTr("Choose how you want to use MEGA")
                    description: qsTr("Choose a installation type")
                    Layout.fillWidth: false
                    Layout.preferredWidth: 488
                    Layout.topMargin: 32
                    Layout.leftMargin: 32
                }

                SyncsInstallationType {
                    id: installationTypeContent

                    Layout.fillWidth: false
                    Layout.preferredWidth: 488
                    Layout.leftMargin: 32
                }

                Connections {
                    target: installationTypeContent

                    onOptionSelected: (option) => {
                        console.log("Selected option -> " + option);
                    }
                }
            }

        } // Component -> installationTypePage

    } //StackView -> configurationStack

} // Item
