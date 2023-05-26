// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts

ColumnLayout {

    property alias title: title.text
    property alias description: description.text

    MegaTexts.RichText {
        id: title

        Layout.preferredHeight: 20
        font.family: "Inter"
        font.styleName: "normal"
        font.weight: Font.DemiBold
        font.pixelSize: MegaTexts.Text.Size.Large
        lineHeight: 30
    }

    MegaTexts.RichText {
        id: description

        Layout.topMargin: 12
        Layout.fillWidth: true
        font.family: "Inter"
        font.styleName: "normal"
        font.weight: Font.Light
        font.pixelSize: MegaTexts.Text.Size.Medium
        wrapMode: Text.WordWrap
    }

}
