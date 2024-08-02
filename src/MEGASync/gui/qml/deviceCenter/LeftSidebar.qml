import QtQuick 2.0
import components.texts 1.0 as Texts
Item {
    id:root

    readonly property int titleMargins: 24

    Rectangle {
        id: contentItem

        anchors.fill: parent
    }
    Texts.RichText {
        id: deviceCenterTitle

        anchors {
            left: parent.left
            top: parent.top
            topMargin: titleMargins
            leftMargin: titleMargins
        }
        wrapMode: Text.NoWrap
        text: DeviceCenterStrings.deviceCentre
        lineHeight: 30
        font {
            pixelSize: Texts.Text.Size.LARGE
            weight: Font.Bold
        }
    }
}
