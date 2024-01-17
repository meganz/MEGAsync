import QtQuick 2.15

import Backups 1.0

ResumePageForm {
    id: root

    footerButtons {

        rightSecondary.onClicked: {
            backupsAccess.openBackupsTabInPreferences();
        }

        rightPrimary.onClicked: {
            window.accept();
        }
    }

}
