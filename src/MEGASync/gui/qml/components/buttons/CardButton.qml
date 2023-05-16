// System
import QtQuick 2.12

// QML common
import Common 1.0
import Components 1.0 as Custom

Button {
    id: button

    property string title: ""
    property string description: ""
    property string imageSource: ""
    property size imageSourceSize
    property Component contentComponent

    checkable: true
    checked: false
    autoExclusive : true

    background: Rectangle {
        id: buttonBackground

        readonly property int borderRadius: 8
        readonly property int borderWidth: 2

        border.width: borderWidth
        radius: borderRadius
        color: Styles.surface1
        border.color: button.checked || button.hovered
                      ? Styles.borderStrongSelected
                      : Styles.borderStrong

        Loader {
            id: contentLoader

            anchors.fill: parent
        }
    }

    onContentComponentChanged: {
        contentLoader.sourceComponent = contentComponent;
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onPressed: {
             mouse.accepted = false;
       }
    }
}
