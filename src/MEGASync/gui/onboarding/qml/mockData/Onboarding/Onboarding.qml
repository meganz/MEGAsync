pragma Singleton
import QtQuick 2.0

Item {

    signal accountBlocked
    signal logout

    function openPreferences(sync) {
        console.info("mockup Onboarding::openPreferences() -> " + sync);
    }

}
