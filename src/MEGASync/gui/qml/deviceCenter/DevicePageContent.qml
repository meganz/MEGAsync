import QtQuick 2.0
import components.texts 1.0 as Texts

Item {
    id:root

    Rectangle {
        id: contentItem

        anchors.fill: parent
        color:"yellow";
    }
    Texts.RichText {
        anchors.centerIn: parent
        wrapMode: Text.NoWrap
        text: "Device Page Content (" + root.width + ", " + root.height+")";
    }
}
