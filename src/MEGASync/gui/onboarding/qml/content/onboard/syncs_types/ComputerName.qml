import QtQuick 2.12
import QtQuick.Layouts 1.12

import Common 1.0
import Components 1.0 as Custom

ColumnLayout {

    onVisibleChanged: {
        if(visible) {
            footerLayout.previousButton.visible = false;
        }
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
        Layout.fillWidth: true
        Layout.preferredHeight: 48
        Layout.leftMargin: -4
        placeholderText: qsTr("MacbookPro Username")
    }

} // RowLayout -> mainLayout
