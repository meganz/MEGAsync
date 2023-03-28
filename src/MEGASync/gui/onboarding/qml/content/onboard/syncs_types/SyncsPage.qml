import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

Page {
    id: mainItem

    function resetSubstackToInitialPage() {
        substackView.replace(substackView.initialItem, StackView.Immediate);
    }

    property var next
    property var previous
    property Footer footerLayout: Footer {}
    property alias substackView: substackView

    onVisibleChanged: {
        if(visible) {
            footerLayout.parentPage = mainItem.objectName;
        }
    }

    StackView {
        id: substackView
    }

    Connections {
        target: footerLayout

        onNextButtonClicked: {
            if(mainItem.visible && next !== undefined) {
                mainItem.StackView.view.replace(next, StackView.Immediate);
            }
        }

        onPreviousButtonClicked: (page) => {
            if(previous !== undefined && previous.next !== undefined && previous.next.objectName === page) {
                mainItem.StackView.view.replace(previous, StackView.Immediate);
            }
        }
    }

    //footer: Footer{}
}
