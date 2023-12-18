import QtQuick 2.15 as Qml

import common 1.0

Text {
    id: root

    readonly property url defaultUrl: "default"
    readonly property int focusMargin: 8

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

            // We are scanning the pixels of the link to look for coordinates with hyperlink.
            var found = false;
            var closed = false;
            var exit = false;
            var link = "";
            var linkCoords = new Object;
            var linkCoordsList = [];
            const verticalLineOffset = 3;

            // we are starting with y to allow multiline text.
            for (var y = 0; y < root.height && !exit; ++y) {
                for (var x = 0; x < root.width && !exit; ++x) {

                    var currentLink = root.linkAt(x, y);
                    if (link.length > 0 && currentLink.length > 0 && link !== currentLink) { // detected second link in literal, not allowed.
                        exit = true;
                        break;
                    }

                    if (currentLink.length > 0 && !found) { // detected a new link in pixel
                        found = true;
                        closed = false;

                        link = currentLink;
                        linkCoords.xiFocus = x;
                        linkCoords.yiFocus = y;
                    }
                    else if (currentLink.length > 0 && found && closed && linkCoords.yiFocus !== y
                             && (linkCoords.yiFocus + font.pixelSize + verticalLineOffset) < y) // link continues in the next line.
                    {
                        linkCoords = new Object;
                        linkCoords.xiFocus = x;
                        linkCoords.yiFocus = y;

                        closed = false;
                    }
                    else if (currentLink.length === 0 && found && !closed) {
                        if (linkCoords.yiFocus !== y) { // we detected the lose of link in the next line
                            linkCoords.xfFocus = root.width;
                        }
                        else {
                            linkCoords.xfFocus = x; // detect the lose of link in current line.
                        }

                        linkCoordsList.push(linkCoords);
                        closed = true;
                    }
                }
            }

            // truly corner case :-) link is located on the edge of the text.
            if (found && !closed) {
                linkCoords.xfFocus = root.width;
                linkCoordsList.push(linkCoords);
            }

            // if found link on text, make focus border visible
            if(found) {
                focusRepeater.model = linkCoordsList.length

                for(var coordsIndex = 0; coordsIndex < linkCoordsList.length; ++coordsIndex) {
                    var coords = linkCoordsList[coordsIndex];

                    var focusX = coords.xiFocus-focusMargin;
                    var focusY = coords.yiFocus-(focusMargin/2);
                    var focusWidth = coords.xfFocus - coords.xiFocus + focusMargin * 2;
                    var focusHeight = root.font.pixelSize + focusMargin + (focusMargin * (3/5));

                    var focusRect = focusRepeater.itemAt(coordsIndex);
                    focusRect.x = focusX;
                    focusRect.y = focusY;
                    focusRect.width = focusWidth;
                    focusRect.height = focusHeight;
                    focusRect.visible = true;
                }
            }
        }
        else {
            for(var modelIndex=0; modelIndex < focusRepeater.model; ++modelIndex) {
                focusRepeater.itemAt(modelIndex).visible = false;
            }
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

    Qml.Keys.onPressed: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
            linkActivated("");
        }
    }

    Qml.Repeater{
        id: focusRepeater

        Qml.Rectangle {
            id: focusBorder

            color: "transparent"
            radius: Sizes.focusBorderRadius
            visible: false
            border {
                color: Styles.focus
                width: Sizes.focusBorderWidth
            }
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
