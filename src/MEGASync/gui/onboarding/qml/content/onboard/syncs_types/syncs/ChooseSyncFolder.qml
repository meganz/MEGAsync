import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import Common 1.0
import Components 1.0 as Custom

ColumnLayout {
    id: root
    property bool local: true
    /*
     * Signals
     */

    /*
     * Object properties
     */
    spacing: 8


    /*
     * Child objects
     */

    Text {
        id: title
        Layout.alignment: Qt.AlignLeft
        text: local ? qsTr("Select a local folder") : qsTr("Select a MEGA folder")
        color: Styles.textColor
        font.pixelSize: 14
        font.bold: true
    }
    RowLayout{
        spacing: 8

        Custom.IconTextField {
            Layout.fillWidth: true
            text: "/MEGA"
            imageSource: local ? "../../../../../../images/onboarding/syncs/pc.svg"
                               : "../../../../../../images/onboarding/syncs/mega.svg"
        }

        Custom.Button {
            id: button
            text: qsTr("Choose")
        }
    }
}


/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
