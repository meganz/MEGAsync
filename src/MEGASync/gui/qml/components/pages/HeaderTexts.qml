import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts

ColumnLayout {
    id: root

    property alias title: titleItem.rawText
    property alias titleWeight: titleItem.font.weight
    property alias description: descriptionItem.text
    property alias descriptionWeight: descriptionItem.font.weight
    property alias descriptionColor: descriptionItem.color
    property alias descriptionFontSize: descriptionItem.font.pixelSize
    property alias titleWrapMode: titleItem.wrapMode

    spacing: (root.title !== "" && root.description !== "") ? 8 : 0

    Texts.RichText {
        id: titleItem

        Layout.fillWidth: true
        font {
            pixelSize: Texts.Text.Size.LARGE
            weight: Font.DemiBold
        }
    }

    Texts.SecondaryText {
        id: descriptionItem

        Layout.fillWidth: true
        font.pixelSize: Texts.Text.Size.MEDIUM
        wrapMode: Text.WordWrap
    }

}
