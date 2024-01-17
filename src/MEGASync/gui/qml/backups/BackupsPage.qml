BackupsFlow {
    id: root

    required property var backupsContentItemRef

    onBackupFlowMoveToBack: {
        window.close();
    }

    onBackupFlowMoveToFinal: (success) => {
        if (success) {
            backupsContentItemRef.state = backupsContentItemRef.resume;
        }
    }

}
