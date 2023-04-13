// System
import QtQuick 2.12
import QtQuick.Controls 2.12

// Local
import Components 1.0 as Custom
import Common 1.0

Custom.TextField {
    id: control
    property url imageSource: imageSource
    property bool readOnly: true
    /*
     * Components
     */

    height: outRect.height
    textField.leftPadding: 16 + image.width
    textField.readOnly: readOnly
    Custom.SvgImage {
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

} // Custom.TextField -> control
