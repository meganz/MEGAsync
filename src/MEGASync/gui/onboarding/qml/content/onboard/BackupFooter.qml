import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Components 1.0 as Custom

RowLayout {
    id: mainLayout

    signal nextButtonClicked
    signal previousButtonClicked

    Custom.Button {
        text: "Previous"

        onClicked: {
            previousButtonClicked()
        }
    }

    Custom.Button {
        text: "Next"

        onClicked: {
            nextButtonClicked()
        }
    }
}
