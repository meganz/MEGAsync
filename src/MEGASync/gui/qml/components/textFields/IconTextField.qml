import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.textFields 1.0
import components.images 1.0

TextField {
    id: control

    property url imageSource: imageSource
    property bool readOnly: true

    height: outRect.height
    textField.leftPadding: 16 + image.width
    textField.readOnly: readOnly

    SvgImage {
        id: image

        source: imageSource
        color: Styles.iconSecondary
        height: control.outRect.height
        anchors{
            left: control.left
            leftMargin: 16
        }

        sourceSize: Qt.size(16 ,16)
    }

}
