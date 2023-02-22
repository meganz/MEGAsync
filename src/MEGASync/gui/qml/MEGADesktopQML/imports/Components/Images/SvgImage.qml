import QtQuick 2.12
import Styles 1.0
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.12


Item
{
    property alias color: overlay.color
    property alias source: image.source
    property alias sourceSize: image.sourceSize

    Image{
        id: image
        anchors.centerIn: parent
    }
    ColorOverlay {
        id: overlay
        source: image
        color: color
        anchors.fill: image
    }
}
