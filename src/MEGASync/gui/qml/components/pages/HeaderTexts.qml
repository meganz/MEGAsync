import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts

Column {
    id: root

    property alias title: titleItem.rawText
    property alias titleWeight: titleItem.font.weight
    property alias description: descriptionItem.text
    property alias descriptionWeight: descriptionItem.font.weight
    property alias descriptionColor: descriptionItem.color
    property alias descriptionFontSize: descriptionItem.font.pixelSize
    property alias titleWrapMode: titleItem.wrapMode

    spacing: (root.title !== "" && root.description !== "") ? 8 : 0
    Layout.fillWidth: true
    Layout.preferredHeight: implicitHeight

    Texts.RichText {
        id: titleItem

        width: root.width
        visible: rawText !== ""
        font {
            pixelSize: Texts.Text.Size.LARGE
            weight: Font.DemiBold
        }
    }

    Texts.SecondaryText {
        id: descriptionItem

        width: root.width
        visible: text !== ""
        font.pixelSize: Texts.Text.Size.MEDIUM
        wrapMode: Text.WordWrap
    }

}
