import QtQuick 2.12
import QtQuick.Layouts 1.12

import Common 1.0
import Components 1.0 as Custom

ColumnLayout {
    id: root

    enum SelectedOption {
        Sync = 0,
        Backup = 1,
        Fuse = 2
    }

    /*
     * Signals
     */

    signal optionSelected(option: int)

    /*
     * Object properties
     */

    width: 488
    spacing: 12

    /*
     * Child objects
     */

    SyncsInstallationTypeButton {
        id: syncButton

        title: qsTr("Sync")
        description: qsTr("Sync your files between your computers with MEGA cloud, any change from one side will apply to another side.")
        imageSource: "../../../../images/Onboarding/sync.svg"

        onSelected: {
            syncsButton.deselect();
            fuseButton.deselect();
            root.optionSelected(SyncsInstallationType.SelectedOption.Sync);
        }
    }

    SyncsInstallationTypeButton {
        id: syncsButton

        title: qsTr("Backup")
        description: qsTr("Automatically update your files from your computers to MEGA cloud. Files in your computer won’t be affected by the cloud.")
        imageSource: "../../../../images/Onboarding/cloud.svg"

        onSelected: {
            syncButton.deselect();
            fuseButton.deselect();
            root.optionSelected(SyncsInstallationType.SelectedOption.Backup);
        }
    }

    SyncsInstallationTypeButton {
        id: fuseButton

        title: qsTr("Fuse")
        description: qsTr("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.")
        imageSource: "../../../../images/Onboarding/fuse.svg"

        onSelected: {
            syncsButton.deselect();
            syncButton.deselect();
            root.optionSelected(SyncsInstallationType.SelectedOption.Fuse);
        }
    }
}

