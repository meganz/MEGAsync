import QtQuick.Layouts 1.15

import common 1.0

import components.buttons 1.0

CardVerticalButton {
    id: root

    property int type: Constants.SyncType.SYNC

    Layout.preferredWidth: width
    Layout.preferredHeight: height
    contentMargin: 24
    contentSpacing: 8
    imageSourceSize: Qt.size(32, 32)
    checkable: false

}


