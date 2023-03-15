import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

RowLayout {
    id: mainLayout

    signal nextButtonClicked
    signal previousButtonClicked

    Button {
        text: "Previous"

        onClicked: {
            previousButtonClicked()
        }
    }

    Button {
        text: "Next"

        onClicked: {
            nextButtonClicked()
        }
    }
}
