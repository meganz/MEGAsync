import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0

ColumnLayout {

    property alias title: title.rawText
    property alias description: description.text
    property alias descriptionWeight: description.font.weight

    spacing: 12

    RichText {
        id: title

        Layout.fillWidth: true
        Layout.preferredHeight: 30
        font.pixelSize: Text.Size.Large
    }

    SecondaryText {
        id: description

        Layout.fillWidth: true
        font.pixelSize: Text.Size.Normal
        wrapMode: Text.WordWrap
        Layout.preferredHeight: 17
    }

}
