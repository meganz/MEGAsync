import QtQuick 2.15

import common 1.0

Button {
    id: root

    property string url
    property bool visited: false

    function openHelpUrl() {
        Qt.openUrlExternally(url);
        visited = true;
    }

    sizes.borderLess: true

    icons {
        colorEnabled: visited ? colorStyle.linkVisited : colorStyle.linkPrimary
        colorHovered: visited ? colorStyle.linkVisited : colorStyle.linkPrimary
        colorPressed: visited ? colorStyle.linkVisited : colorStyle.linkPrimary
    }

    colors {
        background: "transparent"
        disabled: "transparent"
        hover: "transparent"
        border: "transparent"
        borderHover: "transparent"
        borderDisabled: "transparent"
        text: visited ? colorStyle.linkVisited : colorStyle.linkPrimary
        textHover: visited ? colorStyle.linkVisited : colorStyle.linkPrimary
        textPressed: visited ? colorStyle.linkVisited : colorStyle.linkPrimary
        pressed: "transparent"
        borderPressed: "transparent"
    }

    onClicked: {
        openHelpUrl();
    }

    Keys.onPressed: {
        if(event.key === Qt.Key_Space || event.key === Qt.Key_Return) {
            openHelpUrl();
            event.accepted = true;
        }
    }

}

