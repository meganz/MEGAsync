#ifndef BACKUPSWIZARD_H
#define BACKUPSWIZARD_H

#include "model/SyncModel.h"
#include "control/SyncController.h"
#include "megaapi.h"
#include "control/Utilities.h"
#include "HighDpiResize.h"

#include <QDialog>
#include <QList>
#include <QListWidgetItem>
#include <QSemaphore>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

namespace Ui {
class BackupsWizard;
class BackupSetupSuccessDialog;
}

class ProxyModel : public QSortFilterProxyModel
{
public:
    explicit ProxyModel(QObject *parent = nullptr);
    void showOnlyChecked(bool val);
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QModelIndex getIndexByPath(const QString& path);

private:
    bool mShowOnlyChecked;
};

class BackupsWizard : public QDialog
{
        Q_OBJECT

    public:
        enum Steps
        {
            STEP_1_INIT = 0,
            STEP_1,
            STEP_2_INIT,
            STEP_2,
            FINALIZE,
            SETUP_MYBACKUPS_DIR,
            SETUP_BACKUPS,
            DONE,
            ERROR_FOUND,
            EXIT,
        };

        enum Status
        {
           QUEUED = 0,
           ERR,
           OK,
        };

        struct BackupInfo
        {
           QString folderName;
           Status status;
        };

        explicit BackupsWizard(QWidget* parent = nullptr);
        ~BackupsWizard();

    protected:
        void changeEvent(QEvent* event) override;

    private:
        void showLess();
        void showMore();
        void setupStep1();
        void setupStep2();
        void setupError();
        void setupFinalize();
        void setupMyBackupsDir();
        void setupBackups();
        void setupComplete();
        bool isSomethingChecked();
        void processNextBackupSetup();
        bool isFolderAlreadySynced(const QString& path, bool displayWarning = false);
        void refreshNextButtonState();
        void nextStep(const Steps& step);
        void setCurrentWidgetsSteps(QWidget* widget);

        Ui::BackupsWizard* mUi;
        HighDpiResize mHighDpiResize;
        SyncController mSyncController;
        bool mCreateBackupsDir;
        mega::MegaHandle mDeviceDirHandle;
        bool mHaveBackupsDir;
        bool mError;
        bool mUserCancelled;
        QStandardItemModel* mFoldersModel;
        ProxyModel* mFoldersProxyModel;
        QMap<QString, BackupInfo> mBackupsStatus;
        QStringList mErrList;
        QWidget* mLoadingWindow;

    private slots:
        void on_bNext_clicked();
        void on_bCancel_clicked();
        void on_bMoreFolders_clicked();
        void on_bBack_clicked();
        void on_bViewInBackupCentre_clicked();
        void on_bDismiss_clicked();
        void on_bTryAgain_clicked();
        void on_bCancelErr_clicked();
        void on_bShowMore_clicked();

        void onDeviceNameSet(QString deviceName);
        void onBackupsDirSet(mega::MegaHandle backupsDirHandle);
        void onSetMyBackupsDirRequestStatus(int errorCode, const QString& errorMsg);
        void onSyncAddRequestStatus(int errorCode, const QString &errorMsg, const QString &name);
};

#endif // BACKUPSWIZARD_H
