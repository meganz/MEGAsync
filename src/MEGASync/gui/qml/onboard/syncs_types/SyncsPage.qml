import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

Rectangle {
    id: root

    property alias footerButtons: footerButtonsItem

    function setInitialFocusPosition() {
        onboardingWindow.requestPageFocus();
    }

    color: Styles.surface1

    Footer {
        id: footerButtonsItem
    }
}
