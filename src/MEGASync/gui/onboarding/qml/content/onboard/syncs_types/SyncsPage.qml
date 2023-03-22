import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

Page {

    function resetToInitialPage() {
        stackView.replace(stackView.initialItem, StackView.Immediate);
        isFirstPage = true;
    }

    function nextPage() {
        stackView.replace(stackView.currentItem.next, StackView.Immediate);
        isFirstPage = stackView.currentItem == stackView.initialItem;
    }

    function previousPage() {
        stackView.replace(stackView.currentItem.previous, StackView.Immediate);
        isFirstPage = stackView.currentItem == stackView.initialItem;
    }

    property SyncsPage next
    property SyncsPage previous
    property Footer footerLayout: Footer {}
    property bool isStacked: false
    property bool isFirstPage: false

    property alias stackView: stackView

    StackView {
        id: stackView
    }

}
