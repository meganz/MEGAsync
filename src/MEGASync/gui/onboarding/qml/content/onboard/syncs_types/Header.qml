// System
import QtQuick 2.15
import QtQuick.Layouts 1.15

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts

ColumnLayout {

    property alias title: title.rawText
    property alias description: description.text
    property alias descriptionWeight: description.font.weight

    spacing: 12

    MegaTexts.RichText {
        id: title

        Layout.fillWidth: true
        Layout.preferredHeight: 30
        font.pixelSize: MegaTexts.Text.Size.Large
    }

    MegaTexts.SecondaryText {
        id: description

        Layout.fillWidth: true
        font.pixelSize: MegaTexts.Text.Size.Normal
        wrapMode: Text.WordWrap
        Layout.preferredHeight: 17
    }

}
