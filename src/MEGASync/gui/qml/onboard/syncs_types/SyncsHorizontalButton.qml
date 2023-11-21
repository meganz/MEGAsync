// System
import QtQuick.Layouts 1.15

// QML common
import components.buttons 1.0

MegaButtons.CardHorizontalButton {

    property alias type: syncsType.type
    property SyncsType syncs: SyncsType { id: syncsType }

}


