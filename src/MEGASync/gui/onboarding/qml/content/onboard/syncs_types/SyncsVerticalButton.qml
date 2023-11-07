// System
import QtQuick.Layouts 1.15

// QML common
import Components.Buttons 1.0 as MegaButtons

MegaButtons.CardVerticalButton {
    id: root

    property alias type: syncsType.type
    property SyncsType syncs: SyncsType { id: syncsType }

    imageSourceSize: Qt.size(64, 64)
}


