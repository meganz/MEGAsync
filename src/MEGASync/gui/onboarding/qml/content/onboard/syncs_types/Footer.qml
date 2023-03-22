import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Components 1.0 as Custom

RowLayout {
    id: mainLayout

    signal cancelButtonClicked
    signal nextButtonClicked
    signal previousButtonClicked

    property alias cancelButton: cancelButton
    property alias previousButton: previousButton
    property alias nextButton: nextButton
    property alias notNow: notNow

    Text {
        id: notNow

        text: qsTr("Not now")
        font.pixelSize: 12
        font.weight: Font.Light
        font.family: "Inter"
        font.styleName: "Medium"
        font.underline: true
        Layout.leftMargin: 31

        MouseArea {
            anchors.fill: notNow
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                console.debug("TODO: Not now");
            }
        }
    }

    RowLayout {
        Layout.alignment: Qt.AlignRight

        Custom.Button {
            id: cancelButton

            text: qsTr("Cancel")
            onClicked: {
                cancelButtonClicked();
            }
        }

        Custom.Button {
            id: previousButton

            text: qsTr("Previous")
            onClicked: {
                previousButtonClicked();
            }
        }

        Custom.Button {
            id: nextButton

            text: qsTr("Next")
            primary: true
            iconRight: true
            iconSource: "../../../images/Onboarding/arrow_right.svg"
            onClicked: {
                nextButtonClicked();
            }
        }

    }
}
