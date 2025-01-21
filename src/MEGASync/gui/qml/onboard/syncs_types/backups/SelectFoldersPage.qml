import common 1.0

SelectFoldersPageForm {
    id: root

    signal selectFolderMoveToBack
    signal selectFolderMoveToConfirm

    footerButtons {
        leftPrimary.text: Strings.skip
        leftPrimary.onClicked: {
            window.close();
        }

        rightSecondary.onClicked: {
            root.selectFolderMoveToBack();
        }

        rightPrimary.onClicked: {
            root.selectFolderMoveToConfirm();
        }
    }

}
