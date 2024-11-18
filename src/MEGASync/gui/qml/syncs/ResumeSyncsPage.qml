import QtQuick 2.15

import common 1.0

import Syncs 1.0

ResumeSyncsPageForm {
    id: root

    image.source: Images.ok
    image.sourceSize: Qt.size(128, 128)

    footerButtons {

        rightSecondary.onClicked: {
            syncsComponentAccess.openSyncsTabInPreferences();
            window.accept();
        }

        rightPrimary.onClicked: {
            window.accept();
        }
    }

}
