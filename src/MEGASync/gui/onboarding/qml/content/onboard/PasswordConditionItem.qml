// System
import QtQuick 2.15

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

Row {
    id: root

    readonly property int iconWidth: 16

    property alias text: condition.text

    property bool checked: false

    width: parent.width
    spacing: iconWidth / 2

    MegaImages.SvgImage {
        id: image

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
