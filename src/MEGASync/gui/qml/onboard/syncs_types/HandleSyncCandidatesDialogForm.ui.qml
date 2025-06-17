import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.checkBoxes 1.0
import components.pages 1.0

import syncs 1.0

Rectangle {
    id: root

    readonly property int dialogMargin: 32
    readonly property int dialogWidth: 453
    readonly property int dialogHeight: 344
    readonly property int buttonsSpacing: 12
    readonly property int dialogRadius: 10
    readonly property int underTitleMargin: 24
    readonly property int titleLineHeight: 24
    readonly property int minButtonsSize: 80

    property alias localFolderChooser: localFolder
    property alias remoteFolderChooser: remoteFolder
    property alias rightPrimaryButton : acceptButton
    property alias rightSecondaryButton : cancelButton
    property alias dialogTitle: title.text

    color: ColorTheme.surface1
    radius: dialogRadius
    width: dialogWidth
    height: dialogHeight

    Texts.RichText {
        id: title

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: dialogMargin
        anchors.leftMargin: dialogMargin + Constants.focusBorderWidth

        lineHeightMode: Text.FixedHeight
        lineHeight: titleLineHeight
        font {
            pixelSize: Texts.Text.Size.MEDIUM_LARGE
            weight: Font.DemiBold
        }
        text: SyncsStrings.selectiveSyncTitle
    }

    Column {
        id: column

        anchors.top: title.bottom
        anchors.topMargin: underTitleMargin
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: dialogMargin
        spacing: dialogMargin

        ChooseSyncFolder {
            id: localFolder

            title: SyncsStrings.selectLocalFolder
            leftIconSource: Images.pc
        }

        ChooseSyncFolder {
            id: remoteFolder

            title: SyncsStrings.selectMEGAFolder
            leftIconSource: Images.megaOutline
        }
    }

    RowLayout {
        id: buttonRow

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: dialogMargin
        layoutDirection: Qt.RightToLeft

        PrimaryButton {
            id: acceptButton

            sizes.fillWidth: true
            text: qsTr("Add")
            Layout.minimumWidth: minButtonsSize
        }

        OutlineButton {
            id: cancelButton

            text: qsTr("Cancel")
            sizes.fillWidth: true
            visible: true
            Layout.minimumWidth: minButtonsSize
        }
    }
}
