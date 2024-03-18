import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.textFields 1.0
import components.images 1.0

TextField {
    id: root

    property url imageSource
    property bool readOnly: true

    height: outRect.height
    textField {
        leftPadding: 16 + image.width
        readOnly: root.readOnly
    }

    SvgImage {
        id: image

        anchors{
            left: root.left
            leftMargin: 16
        }
        height: root.outRect.height
        sourceSize: Qt.size(16 ,16)
        source: imageSource
        color: Styles.iconSecondary
    }
}
