import QtQuick 2.0

Item {

    signal accountBlocked(int blocked)

    function isAccountBlocked() {
        console.debug("mockup AccountStatusController::isAccountBlocked()");
    }

    function whyAmIBlocked(force) {
        console.debug("mockup AccountStatusController::whyAmIBlocked() : force -> " + force);
    }

}
