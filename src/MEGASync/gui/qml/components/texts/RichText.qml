// System
import QtQuick 2.12

// Local
import Common 1.0

Text {
    id: control

    function updateLinkColor() {
        var color = Styles.linkPrimary;
        if(!enabled) {
            color = Styles.linkInverse;
        } else if(visited) {
            color = Styles.linkVisited;
        }
        control.text = control.text.replace("color:" + urlColor, "color:" + color);
        urlColor = color;
    }

    readonly property url defaultUrl: "default"

    property url url: defaultUrl
    property bool manageMouse: false
    property bool hovered: false
    property bool visited: false
    property color urlColor: Styles.linkPrimary

    onManageMouseChanged: {
        if(manageMouse) {
            loader.sourceComponent = mouseArea;
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
        if(url != defaultUrl) {
            Qt.openUrlExternally(url);
            visited = true;
            updateLinkColor();
        }
    }

    onLinkHovered: {
        hovered = link.length;
    }

    onEnabledChanged: {
        updateLinkColor();
    }

    Loader {
        id: loader

        anchors.fill: parent
    }

    Component {
        id: mouseArea

        MouseArea {
            anchors.fill: parent
            cursorShape: hovered ? Qt.PointingHandCursor : Qt.ArrowCursor
            onPressed: mouse.accepted = false;
        }
    }
}
