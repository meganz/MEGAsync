import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Components 1.0 as Custom
import Onboarding 1.0

RowLayout {

    property alias notNowButton: notNowButton
    property alias previousButton: previousButton
    property alias nextButton: nextButton

    Text {
        id: notNowButton

        text: qsTr("Not now")
        font.pixelSize: 12
        font.weight: Font.Light
        font.family: "Inter"
        font.styleName: "Medium"
        font.underline: true
        Layout.leftMargin: 31

        MouseArea {
            anchors.fill: notNowButton
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                Onboarding.onNotNowClicked();
            }
        }
    }

    RowLayout {
        Layout.alignment: Qt.AlignRight

        Custom.Button {
            id: cancelButton

            text: qsTr("Cancel")
            onClicked: {
                console.log("TODO: Cancel button clicked");
            }
        }

        Custom.Button {
            id: previousButton

            text: qsTr("Previous")
        }

        Custom.Button {
            id: nextButton

            text: qsTr("Next")
            primary: true
            iconRight: true
            iconSource: "../../../images/Onboarding/arrow_right.svg"
        }

    }
}
