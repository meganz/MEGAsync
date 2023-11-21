import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

Rectangle {

    property alias footerButtons: footerButtonsItem

    color: Styles.surface1

    Footer {
        id: footerButtonsItem
    }
}
