import QtQuick 2.15

import Syncs 1.0

ResumeSyncsPageForm {
    id: root

    footerButtons {

        rightSecondary.onClicked: {
            syncsAccess.openSyncsTabInPreferences();
            window.accept();
        }

        rightPrimary.onClicked: {
            window.accept();
        }
    }

}
