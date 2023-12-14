import QtQuick 2.15 as Qml

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0

Texts.Text {
    id: root

    readonly property url defaultUrl: "default"
    readonly property int focusMargin: 6

    property url url: defaultUrl
    property bool manageMouse: false
    property bool hovered: false
    property bool manageHover: false
    property bool visited: false
    property color urlColor: Styles.linkPrimary
    property string rawText: ""

    function updateLinkColor() {
        var color = Styles.linkPrimary;
        if(!enabled) {
            color = Styles.notificationInfo;
        } else if(visited) {
            color = Styles.linkVisited;
        }
        root.text = root.text.replace("color:" + urlColor, "color:" + color);
        urlColor = color;
    }

    function placeFocusBorder() {
        if (root.activeFocus && root.text.length > 0) {
            var xiFocus = -1;
            var xfFocus = -1;
            var yiFocus = -1;
            var yfFocus = -1;

            var found = false;
            var exit = false;
            for (var x = 0; x < root.width && !exit; ++x) {
                for (var y = 0; y < root.height && !exit; y+=root.font.pixelSize) {
                    var result  = root.linkAt(x, y)
                    if (result.length > 0 && !found) {
                        found = true;
                        xiFocus = x;
                        yiFocus = y;
                    }
                    else if (result.length === 0 && found) {
                        exit = true;
                        xfFocus = x;
                    }
                }
            }

            // truly corner case : link is placed on the edge of the text.
            if (found && !exit) {
                xfFocus = text.width;
            }

            focusBorder.x = xiFocus-focusMargin;
            focusBorder.y = yiFocus-focusMargin/2;
            focusBorder.width = xfFocus - xiFocus + focusMargin * 2;
            focusBorder.height = root.font.pixelSize + focusMargin * 2;
            focusBorder.visible = true;
        }
        else {
            focusBorder.visible = false;
        }
    }

    color: enabled ? Styles.textPrimary : Styles.textDisabled
    textFormat: Qml.Text.RichText

    // We are using rawText to avoid breaking internal connections in the text property.
    // If we assign a string directly to the RichText text property and we use the replace
    // javascript function (we modify the text), then when the text is updated, it is not
    // refreshed internally. We cannot assign other variable and change it here at the
    // same time. For more info, please see SNC-3917.
    onRawTextChanged: {
        var copyText = rawText;
        copyText = copyText.replace("[B]","<b>");
        copyText = copyText.replace("[/B]","</b>");
        copyText = copyText.replace("[A]", "<a style=\"text-decoration:none\"
                                            style=\"color:" + urlColor + ";\" href=\"" + url + "\">");
        copyText = copyText.replace("[/A]","</a>");
        root.text = copyText;
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

    onFocusChanged: {
        placeFocusBorder();
    }

    onTextChanged: {
        placeFocusBorder();
    }

    Qml.Rectangle {
        id: focusBorder

        color: "transparent"
        radius: 7
        visible: false
        border {
            color: Styles.focus
            width: 3
        }
    }

    Qml.MouseArea {
        id: mouseArea

        anchors.fill: parent
        cursorShape: hovered ? Qt.PointingHandCursor : Qt.ArrowCursor
        onPressed: { mouse.accepted = false; }
        enabled: root.manageMouse
        hoverEnabled: root.manageHover
    }
}
