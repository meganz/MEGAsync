import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

Page {

    function updateFooter() {
        footerLayout.show(footerState);
    }

    function resetToInitialPage() {
        stackView.replace(stackView.initialItem, StackView.Immediate);
        stackView.currentItem.updateFooter();

        isFirstPage = true;
    }

    function nextPage() {
        stackView.replace(stackView.currentItem.next, StackView.Immediate);
        stackView.currentItem.updateFooter();

        isFirstPage = stackView.currentItem == stackView.initialItem;
    }

    function previousPage() {
        stackView.replace(stackView.currentItem.previous, StackView.Immediate);
        stackView.currentItem.updateFooter();

        isFirstPage = stackView.currentItem == stackView.initialItem;
    }

    property SyncsPage next
    property SyncsPage previous
    property var footerLayout: Footer {}
    property int footerState
    property bool isStacked: false
    property bool isFirstPage: false

    property alias stackView: stackView

    StackView {
        id: stackView
    }

}
