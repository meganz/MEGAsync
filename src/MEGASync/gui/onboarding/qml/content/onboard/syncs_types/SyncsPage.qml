import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml

Qml.Page {

    function updateFooter() {
        footerLayout.show(footerState);
    }

    property SyncsPage next
    property SyncsPage previous
    property var footerLayout: Footer {}
    property int footerState
    property bool isStacked: false
    property bool isFirstPage: false

}
