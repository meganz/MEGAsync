// System
import QtQuick.Layouts 1.12

// QML common
import Components 1.0 as Custom

Custom.CardHorizontalButton {

    property alias type: syncsType.type
    property SyncsType syncs: SyncsType { id: syncsType }

    Layout.preferredWidth: 488
    Layout.preferredHeight: 96
    Layout.fillWidth: true
    width: 488
    height: 96
    imageSourceSize: Qt.size(64, 64)

}


