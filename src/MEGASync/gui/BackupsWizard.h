#ifndef BACKUPSWIZARD_H
#define BACKUPSWIZARD_H

#include "model/SyncModel.h"
#include "control/SyncController.h"
#include "megaapi.h"
#include "RenameTargetFolderDialog.h"
#include "control/Utilities.h"
#include "HighDpiResize.h"

#include <QDialog>
#include <QList>
#include <QListWidgetItem>
#include <QSemaphore>
#include <QStandardItemModel>

namespace Ui {
class BackupsWizard;
class BackupSetupSuccessDialog;
}

class BackupsWizard : public QDialog
{
        Q_OBJECT

    public:
        enum Steps
        {
            STEP_1_INIT = 0,
            STEP_1      = 1,
            STEP_2_INIT = 2,
            STEP_2      = 3,
            FINALIZE    = 4,
            SETUP_MYBACKUPS = 5,
            SETUP_DEVICE_NAME = 6,
            SETUP_BACKUPS   = 7,
            DONE = 8,
            EXIT = 9,
        };
        explicit BackupsWizard(QWidget* parent = nullptr);
        ~BackupsWizard();

    private:
        void setupStep1();
        void setupStep2();
        void setupFinalize();
        void setupMyBackupsDir(bool nameCollision = false);
        void setupDeviceName();
        void setupBackups();
        void setupComplete();
        QString getCurrentState();
        void updateOriginalState(int index);
        bool promptAndEnsureUniqueRemoteName(QString& displayName, bool forcePrompt = false);
        bool isFolderAlreadySynced(const QString& path, bool displayWarning = false);
        QString remoteFolderExistsDialog(const QString& backupName,
                                         RenameTargetFolderDialog::FolderType type,
                                         std::shared_ptr<mega::MegaNode> node = std::shared_ptr<mega::MegaNode>());
        void refreshNextButtonState();
        void displayError(const QString& message);

        Ui::BackupsWizard* mUi;
        HighDpiResize mHighDpiResize;
        Steps mCurrentStep;
        SyncModel* mSyncsModel;
        SyncController mSyncController;
        bool mCreateBackupsDir;
        mega::MegaHandle mDeviceDirHandle;
        QString mBackupsDirName;
        bool mHaveBackupsDir;
        QString mDeviceName;
        bool mHaveDeviceName;
        QString mOriginalState;
        bool mError;
        bool mUserCancelled;
        QStandardItemModel* mStep1FoldersModel;
        int mCurrentSyncIdx;
        std::unique_ptr<QDialog> mSuccessDialog;
        std::unique_ptr<Ui::BackupSetupSuccessDialog> mSuccessDialogUi;

    signals:
        void nextStep();

    private slots:
        void onNextStep();
        void on_bNext_clicked();
        void on_bCancel_clicked();
        void on_bMoreFolders_clicked();
        void on_bBack_clicked();
        void onListItemChanged(QStandardItem* item);
        void onDeviceNameSet(QString deviceName);
        void onBackupsDirSet(mega::MegaHandle backupsDirHandle);
        void onSetMyBackupsDirRequestStatus(int errorCode, QString errorMsg);
        void onSetDeviceNameRequestStatus(int errorCode, QString errorMsg);
        void onSyncAddRequestStatus(int errorCode, QString errorMsg);
        void onSuccessDialogAccepted();
};

#endif // BACKUPSWIZARD_H
