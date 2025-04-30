import QtQuick 2.15

import common 1.0

ResumeSyncsPageForm {
    id: root

    image.source: Images.ok
    image.sourceSize: Qt.size(128, 128)

    footerButtons {

        rightSecondary.onClicked: {
            syncsComponentAccess.viewSyncsInSettingsButtonClicked();
            window.accept();
        }

        rightPrimary.onClicked: {
            window.accept();
        }
    }

}
