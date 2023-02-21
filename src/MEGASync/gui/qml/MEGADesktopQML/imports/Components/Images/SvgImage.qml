import QtQuick 2.12
import Styles 1.0
import QtGraphicalEffects 1.15
import QtQuick.Controls 2.12


Image{
    property alias color: overlay.color
    id: image
    ColorOverlay {
        id: overlay
        source: image
        color: color
        anchors.fill: image
    }
}


