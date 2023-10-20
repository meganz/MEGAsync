// System
import QtQuick 2.15
import QtQuick.Controls 2.15

// Local
import Components.TextFields 1.0 as MegaTextFields
import Components.Images 1.0 as MegaImages
import Common 1.0

MegaTextFields.TextField {
    id: control
    property url imageSource: imageSource
    property bool readOnly: true
    /*
     * Components
     */

    height: outRect.height
    textField.leftPadding: 16 + image.width
    textField.readOnly: readOnly
    MegaImages.SvgImage {
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
