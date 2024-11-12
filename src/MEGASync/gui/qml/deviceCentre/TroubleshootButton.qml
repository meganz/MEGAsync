import QtQuick.Layouts 1.15

import components.buttons 1.0
import components.texts 1.0 as Texts


SecondaryButton {
    id: root

    Layout.alignment: Qt.AlignBottom | Qt.AlignRight
    sizes.textFontSize: Texts.Text.Size.NORMAL

    sizes.horizontalPadding: 8
    sizes.verticalPadding: 4
}
