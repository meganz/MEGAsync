import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Components 1.0 as Custom

SyncsPage {

    property alias computerNameTextField: computerNameTextField

    ColumnLayout {
        anchors.leftMargin: 32
        anchors.left: parent.left
        spacing: 12

        Header {
            title: qsTr("Set up MEGA")
            description: qsTr("You can assign the name for personal use or workgroup membership of this computer.")
            Layout.fillWidth: false
            Layout.preferredWidth: 488
            Layout.topMargin: 32
        }

        Image {
            source: "../../../../../images/Onboarding/pc.svg"
            Layout.topMargin: 12
        }

        Text {
            text: qsTr("Computer name");
            Layout.topMargin: 12
            Layout.preferredHeight: 20
            font.pixelSize: 14
            font.weight: Font.DemiBold
            font.family: "Inter"
            font.styleName: "Medium"
            lineHeight: 20
        }

        Custom.TextField {
            id: computerNameTextField

            Layout.fillWidth: true
            Layout.preferredHeight: 48
            Layout.leftMargin: -4
            enabled: false
        }
    }
}
