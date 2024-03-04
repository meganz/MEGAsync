import QtQuick.Layouts 1.15

import components.buttons 1.0

CardVerticalButton {
    id: root

    property alias type: syncsType.type

    property SyncsType syncs: SyncsType { id: syncsType }

    imageSourceSize: Qt.size(64, 64)
}


