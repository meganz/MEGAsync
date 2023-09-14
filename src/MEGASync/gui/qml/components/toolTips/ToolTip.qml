// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

Qml.ToolTip {
    id: root

    readonly property int tooltipMargin: 6
    property url leftIconSource: ""
    property int contentSpacing: 0

    onLeftIconSourceChanged: {
        if(leftIconSource === "") {
            return;
        }

        contentSpacing = padding;
        leftIconLoader.sourceComponent = leftIcon;
    }

    z: 10
    padding: 4

    background: Rectangle {
        anchors.fill: parent
        color: Styles.buttonPrimary
        radius: 4
    }

    contentItem: RowLayout {
        height: textToolTip.height
        spacing: contentSpacing

        Loader {
            id: leftIconLoader
        }

        MegaTexts.Text {
            id: textToolTip

            text: root.text
            color: Styles.textInverse
            // TODO: Get dialog sizes 800/560 by other way (com.qmldialog.QmlDialog) ???
            Layout.maximumWidth: 800 - leftIconLoader.width - 2 * root.padding
                                 - (leftIconLoader.width > 0 ? 3 : 2) * root.tooltipMargin
            Layout.maximumHeight: 560 - 2 * root.padding - 2 * root.tooltipMargin
            // TODO: Get dialog sizes 800/560 by other way (com.qmldialog.QmlDialog) ???
            wrapMode: Text.Wrap
        }
    }

    Component {
        id: leftIcon

        MegaImages.SvgImage {
            source: leftIconSource
            color: Styles.iconOnColor
            sourceSize: Qt.size(16, 16)
        }
    }

}
