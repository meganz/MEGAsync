// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// Local
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Common 1.0

Row {
    id: root

    property alias icon: hintIcon.source
    property alias title: hintTitle.text
    property alias text: hintText.text

    property HintStyle styles: HintStyle {}
    property int textSize: Text.Size.Normal

    height: visible ? hintTitle.height + hintText.height : 0
    visible: false
    spacing: root.icon !== "" ? 8 : 0

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
        width: parent.width - hintIcon.width - root.spacing
        spacing: (hintTitle.height > 0 && hintText.height > 0) ? 2 : 0

        MegaTexts.Text {
            id: hintTitle
            color: styles.titleColor
            opacity: enabled ? 1.0 : 0.2
            font.bold: true
            font.pixelSize: textSize
        }

        MegaTexts.Text {
            id: hintText
            color: styles.textColor
            opacity: enabled ? 1.0 : 0.2
            font.pixelSize: textSize
        }
    }
}
