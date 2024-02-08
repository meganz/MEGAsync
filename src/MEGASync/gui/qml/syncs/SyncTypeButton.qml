import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.buttons 1.0

CardVerticalButton {
    id: root

    property int type: Constants.SyncType.SYNC

    Layout.preferredWidth: width
    Layout.preferredHeight: height
    width: 196
    imageSourceSize: Qt.size(172, 100)
}
