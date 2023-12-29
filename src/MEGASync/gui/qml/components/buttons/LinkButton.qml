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
        colorEnabled: visited ? Styles.linkVisited : Styles.linkPrimary
        colorHovered: visited ? Styles.linkVisited : Styles.linkPrimary
        colorPressed: visited ? Styles.linkVisited : Styles.linkPrimary
    }

    colors {
        background: "transparent"
        disabled: "transparent"
        hover: "transparent"
        border: "transparent"
        borderHover: "transparent"
        borderDisabled: "transparent"
        text: visited ? Styles.linkVisited : Styles.linkPrimary
        textHover: visited ? Styles.linkVisited : Styles.linkPrimary
        textPressed: visited ? Styles.linkVisited : Styles.linkPrimary
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

