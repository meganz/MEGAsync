// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0
import Components 1.0 as Custom

// C++
import Onboard 1.0

Rectangle {

    property alias buttonGroup: buttonGroup
    property alias preferencesButton: preferencesButton
    property alias doneButton: doneButton

    property string title: "Your Sync is set up!"
    property string description: "Lorem ipsum dolor a text that congratulates the user and suggests other options to choose below. Use two lines at most. In this case we offer syncs as an option again."

    color: Styles.surface1

    ColumnLayout {
        anchors.fill: parent

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
            source: Images.resume
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
            color: "transparent"

            ButtonGroup {
                id: buttonGroup
            }

            RowLayout {
                spacing: 11
                anchors.fill: parent

                ResumeButton {
                    id: syncButton

                    title: OnboardingStrings.sync
                    description: OnboardingStrings.syncButtonDescription
                    imageSource: Images.sync
                    ButtonGroup.group: buttonGroup
                    type: InstallationTypeButton.Type.Sync
                    checkable: false
                    imageSourceSize: Qt.size(64, 64)
                }

                ResumeButton {
                    id: backupsButton

                    title: OnboardingStrings.backup
                    description: OnboardingStrings.backupButtonDescription
                    imageSource: Images.cloud
                    ButtonGroup.group: buttonGroup
                    type: InstallationTypeButton.Type.Backup
                    checkable: false
                    imageSourceSize: Qt.size(64, 64)
                }

                ResumeButton {
                    id: fuseButton

                    title: OnboardingStrings.fuse
                    description: OnboardingStrings.fuseButtonDescription
                    imageSource: Images.fuse
                    ButtonGroup.group: buttonGroup
                    type: InstallationTypeButton.Type.Fuse
                    checkable: false
                    imageSourceSize: Qt.size(64, 64)
                }
            }
        }

        RowLayout {
            spacing: 8
            Layout.rightMargin: 32
            Layout.bottomMargin: 24
            Layout.alignment: Qt.AlignBottom | Qt.AlignRight

            Custom.Button {
                id: preferencesButton

                text: OnboardingStrings.openInPreferences
            }

            Custom.Button {
                id: doneButton

                text: OnboardingStrings.done
                primary: true
            }
        }
    }
}
