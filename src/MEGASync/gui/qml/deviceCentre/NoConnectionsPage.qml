import QtQuick 2.0

import components.texts 1.0 as Texts

Item {
    id:root

    anchors.fill: parent

    Rectangle {
        id: contentItem

        anchors.fill: parent
        color:"steelblue";
    }

    Texts.RichText {
        anchors.centerIn: parent
        wrapMode: Text.NoWrap
        text: "No Connections (" + root.width + ", " + root.height+")";
    }
}
