// System
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

Button {
    id: syncButton

    property string title: ""
    property string description: ""
    property string imageSource: ""

    Layout.preferredWidth: parent.width
    Layout.fillWidth: true
    Layout.preferredHeight: 96
    autoExclusive : true

    background: buttonBackground

    Rectangle {
        id: buttonBackground

        border.width: 2
        radius: 8
        color: Styles.surface1
        border.color: syncButton.checked ? Styles.borderStrongSelected : Styles.borderStrong

        ColumnLayout {
            anchors.fill: parent
            spacing: 16

            MegaImages.SvgImage {
                color: syncButton.checked ? Styles.iconAccent : Styles.iconSecondary
                Layout.leftMargin: 24
                source: imageSource

                MegaTexts.Text {
                    text: title
                    Layout.preferredHeight: 24
                    font.pixelSize: MegaTexts.Text.Size.MediumLarge
                    font.weight: Font.Bold
                }

                MegaTexts.Text {
                    text: description
                    lineHeightMode: Text.FixedHeight
                    Layout.preferredWidth: 324
                    Layout.preferredHeight: 32
                    font.pixelSize: MegaTexts.Text.Size.Small
                }
            }
        }
    }

    MouseArea {
        anchors.fill: syncButton
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: syncButton.checked
    }

}
