// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Components 1.0
import Common 1.0

Qml.Button {
    id: button

    property string url
    property size iconSize: text.length === 0 ? Qt.size(24, 24) : Qt.size(16, 16)

    padding: 0

    contentItem: RowLayout {
        spacing: button.text.length === 0 ? 0 : 6

        SvgImage {
            source: "../../../images/Onboarding/help-circle.svg"
            color: button.text.length === 0 ? Styles.buttonPrimary : Styles.linkPrimary
            sourceSize: button.iconSize
        }

        Text {
            color: Styles.linkPrimary
            text: button.text
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font {
                pixelSize: 14
                weight: Font.Light
                family: "Inter"
                styleName: "Medium"
            }
        }
    }

    background: Rectangle {
        color: "transparent"
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: {
            Qt.openUrlExternally(url);
        }
    }
}
