// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// Local
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Common 1.0

Item {
    id: root

    property alias icon: hintIcon.source
    property alias title: hintTitle.text
    property alias text: hintText.text

    property HintStyle styles: HintStyle {}
    property int textSize: Text.Size.Normal

    implicitHeight: row.height

    Row {
        id: row

        height: visible ? col.implicitHeight : 0
        spacing: root.icon !== "" ? 8 : 0
        width: root.width

        MegaImages.SvgImage {
            id: hintIcon

            color: styles.iconColor
            sourceSize: Qt.size(16, 16)
            opacity: enabled ? 1.0 : 0.2
        }

        Column {
            id: col

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: row.width - hintIcon.width - row.spacing

            MegaTexts.Text {
                id: hintTitle

                width: parent.width
                height: text !== "" ? implicitHeight : 0
                color: styles.titleColor
                opacity: enabled ? 1.0 : 0.2
                font.bold: true
                font.pixelSize: textSize
                wrapMode: Text.WordWrap
            }

            MegaTexts.Text {
                id: hintText

                width: parent.width
                height: text !== "" ? implicitHeight : 0
                color: styles.textColor
                opacity: enabled ? 1.0 : 0.2
                font.pixelSize: textSize
                wrapMode: Text.WordWrap
            }
        }
    }
}
