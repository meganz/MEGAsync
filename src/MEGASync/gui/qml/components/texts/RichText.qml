import QtQuick 2.15 as Qml

import common 1.0

Text {
    id: root

    readonly property url defaultUrl: "default"
    readonly property int focusMargin: 8

    property url url: defaultUrl
    property bool manageMouse: false
    property bool manageHover: false
    property bool manageClick: false
    property bool visited: false
    property var visitedLinks: ({})
    property bool underlineLink: false
    property color urlColor: linkColorFor(url.toString())
    property string rawText: ""

    signal linkClicked

    function isVisited(href) {
        return visited || (href !== undefined && visitedLinks[href.toString()] === true);
    }

    function linkColorFor(href) {
        if (!enabled)
            return ColorTheme.notificationInfo
        if (isVisited(href))
            return ColorTheme.linkVisited
        return ColorTheme.linkPrimary
    }

    function markVisited(href) {
        if (href === undefined || href === "")
            return;

        // Reassign a fresh object so the visitedLinks change is observed and the
        // text binding re-renders with the updated per-link colour. The sentinel
        // defaultUrl is marked too, so a single manageClick link (which carries no
        // real href) still turns visited after a click, as it did before.
        let updated = Object.assign({}, visitedLinks);
        updated[href.toString()] = true;
        visitedLinks = updated;
    }

    function renderText() {
        if (rawText.indexOf("[") === -1 && rawText.indexOf("<a") === -1)
                return rawText;

        let decoration = underlineLink ? "underline" : "none";
        let copyText = rawText;

        const replacements = [
            { pattern: /\[B\]/g, replacement: "<b>" },
            { pattern: /\[\/B\]/g, replacement: "</b>" },
            {
                pattern: /\[A\]/g,
                replacement: "<a style=\"text-decoration:" + decoration
                             + "; color:" + linkColorFor(url.toString())
                             + ";\" href=\"" + url + "\">"
            },
            { pattern: /\[\/A\]/g, replacement: "</a>" },
            { pattern: /\[BR\]/g, replacement: "<br>" },
            { pattern: /\[\/BR\]/g, replacement: "" }
        ];

        replacements.forEach(replacement => {
            copyText = copyText.replace(replacement.pattern, replacement.replacement);
        });

        // Colour each bare <a href> by its own visited state, so clicking one
        // link in a multi-link literal does not recolour the others.
        copyText = copyText.replace(
            RegexExpressions.linkWithHrefWithoutStyle,
            (match, hrefGroup, tailGroup) => {
                const hrefMatch = hrefGroup.match(/href="([^"]+)"/);
                const href = hrefMatch ? hrefMatch[1] : "";
                return "<a " + hrefGroup + tailGroup + " style=\"text-decoration:"
                       + decoration + "; color:" + linkColorFor(href) + ";\">";
            }
        );

        return copyText;
    }

    function hasLink() {
        return root.rawText.search("[A]") != -1 || root.rawText.search("<a") != -1;
    }

    function placeFocusBorder() {
        if (root.activeFocus && hasLink()) {

            // we are scanning the pixels of the link to look for coordinates with hyperlink.
            var found = false;
            var closed = false;
            var exit = false;
            var link = "";
            var linkCoordsList = [];

            const linkCoordBluePrint = {
                x: 0,
                y: 0,
                width: 0,
                height: fontMetrics.height
            };

            var linkCoords = Object.create(linkCoordBluePrint);

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
                        linkCoords.x = x;
                        linkCoords.y = y;
                    }
                    else if (currentLink.length > 0 && found && closed && linkCoords.y !== y
                             && (linkCoords.x > x || (linkCoords.x + linkCoords.width) < x)) // link continues in the next line.
                    {
                        linkCoords = Object.create(linkCoordBluePrint);
                        linkCoords.x = x;
                        linkCoords.y = y;

                        closed = false;
                    }
                    else if (currentLink.length === 0 && found && !closed) {
                        if (linkCoords.y !== y) { // we detect the lose of link in the next line
                            linkCoords.width = root.width - linkCoords.x;
                        }
                        else {
                            linkCoords.width = x - linkCoords.x; // we detect the lose of link in the same line
                        }

                        linkCoordsList.push(linkCoords);
                        closed = true;
                    }
                }
            }

            // truly corner case :-) link is located on the edge of the text, last line & end of line.
            if (found && !closed) {
                linkCoords.width = root.width - linkCoords.x;
                linkCoordsList.push(linkCoords);
            }

            // if found link on text, make focus border visible
            if (found) {
                focusRepeater.model = linkCoordsList.length;

                for(var coordsIndex = 0; coordsIndex < linkCoordsList.length; ++coordsIndex) {
                    var coords = linkCoordsList[coordsIndex];

                    var focusRect = focusRepeater.itemAt(coordsIndex);
                    focusRect.x = coords.x - focusMargin;
                    focusRect.y = coords.y - focusMargin / 2; // by convention (design) we are using half of the margin for the top.
                    focusRect.width = coords.width + focusMargin * 2;
                    focusRect.height = coords.height + focusMargin;
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

    color: enabled ? ColorTheme.textPrimary : ColorTheme.textDisabled
    text: renderText()
    textFormat: Qml.Text.RichText

    onLinkActivated: (clickedUrl) => {
        if((clickedUrl !== "" && clickedUrl !== root.defaultUrl) || manageClick) {
            if(!manageClick) {
                Qt.openUrlExternally(clickedUrl);
            }
            else {
                linkClicked();
            }
            let target = (clickedUrl !== "" && clickedUrl !== root.defaultUrl)
                         ? clickedUrl : root.url;
            markVisited(target);
        }
    }

    onActiveFocusChanged: {
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

    Qml.FontMetrics {
        id: fontMetrics

        font: root.font
    }

    Qml.Repeater{
        id: focusRepeater

        Qml.Rectangle {
            id: focusBorder

            color: "transparent"
            radius: Sizes.focusBorderRadius
            visible: false
            border {
                color: ColorTheme.focusColor
                width: Sizes.focusBorderWidth
            }
        }
    }

    textMouseArea {
        cursorShape: (root.hoveredLink.length > 0) ? Qt.PointingHandCursor : Qt.ArrowCursor
        enabled: root.manageMouse
        hoverEnabled: root.manageHover
    }
}
