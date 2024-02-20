import QtQuick 2.15

import common 1.0

Rectangle {
    id: root

    property alias footerButtons: footerButtonsItem

    function setInitialFocusPosition() {
        window.requestPageFocus();
    }

    color: colorStyle.surface1

    FooterButtons {
        id: footerButtonsItem
    }

}
