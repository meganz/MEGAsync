// System
import QtQuick 2.12

// Local
import Common 1.0

Text {
    id: control

    property url url: "default"
    property color urlColor: Styles.linkPrimary
    property bool manageMouse: false
    property bool hovered: false

    onManageMouseChanged:
    {
        if(manageMouse)
        {
            loader.sourceComponent = mouseArea
        }
    }
    color: enabled ? Styles.textPrimary : Styles.textDisabled
    textFormat: Text.RichText
    font.pixelSize: 14

    Component.onCompleted: {
        control.text = control.text.replace("[b]","<b>")
        control.text = control.text.replace("[/b]","</b>")
        control.text = control.text.replace("[a]","<b>
                                            <a style=\"text-decoration:none\"
                                            style=\"color:" + urlColor + ";\" href=\"" + url + "\">")
        control.text = control.text.replace("[/a]","</a></b></font>")
    }

    onLinkActivated: {
        if(url != "default") {
            Qt.openUrlExternally(url);
        }
    }

    onLinkHovered:
    {
        hovered = link.length
    }

    Loader{
        id: loader
        anchors.fill: parent
    }

    Component {
        id: mouseArea
        MouseArea {
            anchors.fill: parent
            cursorShape: hovered ? Qt.PointingHandCursor : Qt.ArrowCursor
            onPressed: mouse.accepted = false
        }
    }
}
