import QtQuick 2.15

import common 1.0

ResumePageForm {
    id: root

    footerButtons {
        rightSecondary.onClicked: {
            backupsComponentAccess.openBackupsTabInPreferences();
            window.accept();
        }

        rightPrimary.onClicked: {
            window.accept();
        }
    }

}
