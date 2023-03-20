import QtQuick 2.12
import QtQuick.Controls 2.12

ResumePageForm {

    function clear() {
        if(buttonGroup.checkState === Qt.PartiallyChecked) {
            buttonGroup.checkState = Qt.Unchecked;
        }
    }

    buttonGroup.onCheckStateChanged: {
        if(buttonGroup.checkedButton != null) {
            optionChanged(buttonGroup.checkedButton.type, buttonGroup.checkState);
        }
    }

    buttonGroup.onCheckedButtonChanged: {
        if(buttonGroup.checkedButton != null) {
            optionChanged(buttonGroup.checkedButton.type, buttonGroup.checkState);
        }
    }

    preferencesButton.onClicked: {
        console.debug("TODO: Open in preferences button clicked");
    }

    doneButton.onClicked: {
        console.debug("TODO: Done button clicked");
    }
}
