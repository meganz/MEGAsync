import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

ContextMenuItem {
    id: root

    enum Position {
        FIRST = 0,
        INTER = 1,
        LAST = 2
    }


    property int position: MenuItem.Position.INTER
    property Colors colors: Colors {
        itemBackgroundHover: colorStyle.textInverse
    }
    property Sizes sizes: Sizes {
        horizontalPadding: 8
        verticalPadding: 8
        focusRadius: 4
    }

    width: 200
    height: root.position === MenuItem.Position.FIRST || position === MenuItem.Position.LAST ? sizes.itemHeight + sizes.verticalPadding : sizes.itemHeight
    leftPadding: sizes.horizontalPadding
    rightPadding: sizes.horizontalPadding
    topPadding: root.position === MenuItem.Position.FIRST ? sizes.verticalPadding : 0
    bottomPadding: root.position === MenuItem.Position.LAST ? sizes.verticalPadding : 0
}
