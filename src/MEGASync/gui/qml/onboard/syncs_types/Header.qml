import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts

ColumnLayout {

    property alias title: title.rawText
    property alias description: description.text
    property alias descriptionWeight: description.font.weight

    spacing: 12

    Texts.RichText {
        id: title

        Layout.fillWidth: true
        Layout.preferredHeight: 30
        font.pixelSize: Texts.Text.Size.Large
        font.weight: Font.DemiBold
    }

    Texts.SecondaryText {
        id: description

        Layout.fillWidth: true
        font.pixelSize: Texts.Text.Size.Normal
        wrapMode: Text.WordWrap
        Layout.preferredHeight: 17
    }

}
