import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Onboard.Syncs_types.Left_panel 1.0
import Components 1.0 as Custom
import Common 1.0

StackView {
    id: syncsStack

    /*
     * Object properties
     */

    height: 520
    width: 776
    initialItem: configurationLayout

    /*
     * Child objects
     */

    RowLayout {
        id: configurationLayout

        /*
         * Object properties
         */

        height: parent.height
        width: parent.width

        /*
         * Child objects
         */

        StepPanel {
            id: syncsInfoStepPanel

            Layout.preferredHeight: configurationLayout.height
            Layout.preferredWidth: 224
        }

        Rectangle {
            color: "#FAFAFB"
            Layout.preferredHeight: configurationLayout.height
            Layout.preferredWidth: configurationLayout.width - syncsInfoStepPanel.width

            ColumnLayout {
                height: parent.height
                width: parent.width

                ContentPanel {
                    id: contentStack

                    footerLayout: syncsFooter
                }

                Footer {
                    id: syncsFooter

                    Layout.alignment: Qt.AlignBottom | Qt.AlignRight
                    Layout.bottomMargin: 24
                    Layout.rightMargin: 32
                    Layout.preferredWidth: parent.width
                }
            }
        }

        Connections {
            target: syncsFooter

            onNextButtonClicked: {
                if(syncsInfoStepPanel.next()) {
                    contentStack.nextPage();
                } else {
                    resumePage.clear();
                    syncsStack.replace(resumePage, StackView.Immediate);
                }
            }

            onPreviousButtonClicked: {
                syncsInfoStepPanel.previous();
                contentStack.previousPage();
            }
        }

    } // RowLayout -> configurationLayout

    ResumePage {
        id: resumePage

        height: parent.height
        width: parent.width
        visible: false
    }

    Connections {
        target: resumePage

        onOptionChanged: (type, checked) => {
            if(checked) {
                syncsStack.replace(configurationLayout, StackView.Immediate);
                syncsInfoStepPanel.changeToStep31();
                contentStack.showPage(type);
            }
        }
    }

} // StackView
