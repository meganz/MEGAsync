import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import Common 1.0
import Components 1.0 as Custom

ColumnLayout {

    /*
     * Properties
     */

    property alias buttonGroup: buttonGroup
    property alias preferencesButton: preferencesButton
    property alias doneButton: doneButton

    property string title: "Your Sync is set up!"
    property string description: "Lorem ipsum dolor a text that congratulates the user and suggests other options to choose below. Use two lines at most. In this case we offer syncs as an option again."

    /*
     * Signals
     */

    signal optionChanged(int type, bool checked)

    /*
     * Child objects
     */

    Text {
        text: title
        color: Styles.textAccent
        Layout.topMargin: 32
        font.pixelSize: 20
        Layout.preferredWidth: parent.width
        font.weight: Font.Bold
        font.family: "Inter"
        font.styleName: "normal"
        horizontalAlignment: Text.AlignHCenter
    }

    Custom.SvgImage {
        source: "../../../../../images/Onboarding/image-02.svg"
        Layout.topMargin: 40
        Layout.alignment: Qt.AlignHCenter
    }

    Text {
        text: description
        color: Styles.textPrimary
        Layout.topMargin: 40
        Layout.preferredHeight: 40
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: 712
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 14
        font.weight: Font.Light
        font.family: "Inter"
        font.styleName: "normal"
        wrapMode: Text.WordWrap
    }

    Rectangle {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: 208
        Layout.preferredWidth: 712

        ButtonGroup {
            id: buttonGroup
        }

        RowLayout {
            spacing: 11
            anchors.fill: parent

            ResumeButton {
                id: syncButton

                title: qsTr("Sync")
                description: qsTr("Sync your files between your computers with MEGA cloud, any change from one side will apply to another side.")
                imageSource: "../../../../../images/Onboarding/sync.svg"
                ButtonGroup.group: buttonGroup
                type: InstallationTypeButton.Type.Sync
            }

            ResumeButton {
                id: backupsButton

                title: qsTr("Backup")
                description: qsTr("Automatically update your files from your computers to MEGA cloud. Files in your computer won’t be affected by the cloud.")
                imageSource: "../../../../../images/Onboarding/cloud.svg"
                ButtonGroup.group: buttonGroup
                type: InstallationTypeButton.Type.Backup
            }

            ResumeButton {
                id: fuseButton

                title: qsTr("Fuse")
                description: qsTr("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.")
                imageSource: "../../../../../images/Onboarding/fuse.svg"
                ButtonGroup.group: buttonGroup
                type: InstallationTypeButton.Type.Fuse
            }
        }

    } // Rectangle

    RowLayout {
        spacing: 8
        Layout.rightMargin: 32
        Layout.bottomMargin: 24
        Layout.alignment: Qt.AlignBottom | Qt.AlignRight

        Custom.Button {
            id: preferencesButton

            text: qsTr("Open in Preferences")
        }

        Custom.Button {
            id: doneButton

            text: qsTr("Done")
            primary: true
        }

    } // RowLayout

} // ColumnLayout
