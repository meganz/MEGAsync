// System
import QtQuick 2.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

Row {

    property alias text: condition.text

    property bool checked: false

    readonly property int iconWidth: 16

    width: parent.width
    spacing: iconWidth / 2

    MegaImages.SvgImage {
        color: checked ? Styles.indicatorGreen : Styles.textSecondary
        source: checked ? Images.check : Images.smallCircle
        sourceSize: Qt.size(iconWidth, iconWidth)
    }

    MegaTexts.SecondaryText {
        id: condition

        width: parent.width - iconWidth
        font.strikeout: checked
    }

}
