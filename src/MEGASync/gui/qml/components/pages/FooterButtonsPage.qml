import QtQuick 2.15

import common 1.0

Item {
    id: root

    property alias footerButtons: footerButtonsItem

    function setInitialFocusPosition() {
        window.requestPageFocus();
    }

    FooterButtons {
        id: footerButtonsItem
    }

}
