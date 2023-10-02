// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts

ColumnLayout {

    property alias title: title.rawText
    property alias description: description.rawText
    property alias descriptionWeight: description.font.weight

    spacing: 12

    MegaTexts.RichText {
        id: title

        Layout.fillWidth: true
        Layout.preferredHeight: 20
        font.family: "Inter"
        font.styleName: "normal"
        font.weight: Font.DemiBold
        font.pixelSize: MegaTexts.Text.Size.Large
    }

    MegaTexts.RichText {
        id: description

        Layout.fillWidth: true
        font.family: "Inter"
        font.styleName: "normal"
        font.pixelSize: MegaTexts.Text.Size.Medium
        wrapMode: Text.WordWrap
    }

}
