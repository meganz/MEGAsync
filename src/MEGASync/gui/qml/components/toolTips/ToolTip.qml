// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Common 1.0
import Components 1.0 as Custom

Qml.ToolTip {
    id: root

    property alias leftIcon: leftIcon

    z: 10

    background: Rectangle {
        color: Styles.buttonPrimary
        radius: 4
    }

    contentItem: RowLayout {
        spacing: leftIcon.source == "" ? 0 : 4
        anchors.margins: 4

        Custom.SvgImage {
            id: leftIcon

            visible: source != ""
            color: Styles.iconOnColor
            sourceSize: Qt.size(16, 16)
        }

        Text {
            text: root.text
            color: Styles.textInverse
            Layout.leftMargin: 4
            Layout.preferredHeight: 16
            font {
                pixelSize: 12
                weight: Font.Light
                family: "Inter"
                styleName: "Medium"
            }
        }
    }
}
