// System
import QtQuick.Layouts 1.12

// QML common
import Components.Buttons 1.0 as MegaButtons

MegaButtons.CardVerticalButton {

    property alias type: syncsType.type
    property SyncsType syncs: SyncsType { id: syncsType }

    Layout.preferredWidth: 230
    Layout.preferredHeight: 208
    width: 230
    height: 208
    imageSourceSize: Qt.size(64, 64)

}


