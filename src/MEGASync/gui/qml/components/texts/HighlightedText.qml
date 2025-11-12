import QtQuick 2.15
import QtQuick.Layouts 1.15

import components.images 1.0
import components.texts 1.0 as Texts


Rectangle {
    id: mainRect

    property alias text: richText.text
    property alias font: richText.font
    property alias textColor: richText.color
    property alias backgroundColor: mainRect.color
    property alias iconSource: iconImage.source
    property alias iconSize: iconImage.sourceSize
    property alias iconColor: iconImage.color
    property alias radius: mainRect.radius

    property int iconTextSpacing: 4
    property int horizontalPadding: 4
    property int verticalPadding: 2
    property int maximumAllowedWidth: -1  // -1 means no limit

    implicitWidth: layout.implicitWidth + 2 * horizontalPadding
    implicitHeight: layout.implicitHeight + 2 * verticalPadding

    width: maximumAllowedWidth > 0 ? Math.min(implicitWidth, maximumAllowedWidth) : implicitWidth

    anchors.leftMargin: horizontalPadding
    anchors.rightMargin: horizontalPadding
    anchors.topMargin: verticalPadding
    anchors.bottomMargin: verticalPadding

    RowLayout {
        id: layout

        spacing: iconTextSpacing
        anchors.centerIn: parent
        width: parent.width - 2 * horizontalPadding

        SvgImage {
            id: iconImage

            sourceSize: Qt.size(16, 16)
            visible: mainRect.visible
        }

        Texts.ElidedText {
            id: richText
            visible: mainRect.visible
            wrapMode: Text.NoWrap
            elide: Qt.ElideMiddle
            verticalAlignment: Text.AlignVCenter
            Layout.fillWidth: true
            Layout.maximumWidth: maximumAllowedWidth > 0 ?
                maximumAllowedWidth - iconImage.width - iconTextSpacing - 2 * horizontalPadding :
                Number.POSITIVE_INFINITY
        }
    }
}
