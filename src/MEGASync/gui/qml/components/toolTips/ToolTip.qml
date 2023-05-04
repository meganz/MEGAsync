// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Common 1.0
import Components 1.0 as Custom

Qml.ToolTip {
    id: root

    property url leftIconSource: ""
    property int contentSpacing: 0

    onLeftIconSourceChanged: {
        if(leftIconSource === "") {
            return;
        }

        contentSpacing = 4;
        leftIconLoader.sourceComponent = leftIcon;
    }

    z: 10
    padding: 4

    background: Rectangle {
        height: parent.height
        color: Styles.buttonPrimary
        radius: 4
    }

    contentItem: RowLayout {
        height: textToolTip.height
        spacing: contentSpacing

        Loader {
            id: leftIconLoader
        }

        Text {
            id: textToolTip

            text: root.text
            color: Styles.textInverse
            Layout.leftMargin: 4
            // TODO: Get dialog sizes 800/560 by other way (com.qmldialog.QmlDialog) ???
            Layout.maximumWidth: 800 - leftIconLoader.width - 2 * root.padding
            Layout.maximumHeight: 560 - 2 * root.padding
            // TODO: Get dialog sizes 800/560 by other way (com.qmldialog.QmlDialog) ???
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font {
                pixelSize: 12
                weight: Font.Light
                family: "Inter"
                styleName: "Medium"
            }
        }
    }

    Component {
        id: leftIcon

        Custom.SvgImage {
            source: leftIconSource
            color: Styles.iconOnColor
            sourceSize: Qt.size(16, 16)
        }
    }

}
