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

        MegaTexts.Text {
            id: textToolTip

            text: root.text
            color: Styles.textInverse
            // TODO: Get dialog sizes 800/560 by other way (com.qmldialog.QmlDialog) ???
            Layout.maximumWidth: 800 - leftIconLoader.width - 2 * root.padding
            Layout.maximumHeight: 560 - 2 * root.padding
            // TODO: Get dialog sizes 800/560 by other way (com.qmldialog.QmlDialog) ???
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.weight: Font.Light
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
