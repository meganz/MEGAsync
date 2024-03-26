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
        colorEnabled: visited ? ColorTheme.linkVisited : ColorTheme.linkPrimary
        colorHovered: visited ? ColorTheme.linkVisited : ColorTheme.linkPrimary
        colorPressed: visited ? ColorTheme.linkVisited : ColorTheme.linkPrimary
    }

    colors {
        background: "transparent"
        disabled: "transparent"
        hover: "transparent"
        border: "transparent"
        borderHover: "transparent"
        borderDisabled: "transparent"
        text: visited ? ColorTheme.linkVisited : ColorTheme.linkPrimary
        textHover: visited ? ColorTheme.linkVisited : ColorTheme.linkPrimary
        textPressed: visited ? ColorTheme.linkVisited : ColorTheme.linkPrimary
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

