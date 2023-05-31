// System
import QtQuick.Layouts 1.12

// QML common
import Components.Buttons 1.0 as MegaButtons

MegaButtons.CardHorizontalButton {

    property alias type: syncsType.type
    property SyncsType syncs: SyncsType { id: syncsType }
}


