#ifndef BACKUPSWIZARD_H
#define BACKUPSWIZARD_H

#include "model/SyncModel.h"
#include "control/SyncController.h"
#include "megaapi.h"
#include "control/Utilities.h"
#include "HighDpiResize.h"
#include "Backups/BackupNameConflictDialog.h"

#include <QDialog>
#include <QList>
#include <QListWidgetItem>
#include <QSemaphore>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

namespace Ui {
class BackupsWizard;
}

namespace UserAttributes
{
    class DeviceName;
    class MyBackupsHandle;
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
        enum Step
        {
            STEP_1_INIT = 0,
            STEP_1,
            STEP_2_INIT,
            STEP_2,
            HANDLE_NAME_CONFLICTS,
            FINALIZE,
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
            Status  status;
            QString syncName;
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
        void handleNameConflicts();
        void setupFinalize();
        void setupBackups();
        void setupComplete();
        bool atLeastOneFolderChecked();
        void processNextBackupSetup();
        bool isFolderSyncable(const QString& path, bool displayWarning = false, bool fromCheckAction = false);
        void nextStep(const Step& step);
        void setCurrentWidgetsSteps(QWidget* widget);
        void updateSize();

        Ui::BackupsWizard* mUi;
        HighDpiResize mHighDpiResize;
        std::shared_ptr<UserAttributes::DeviceName> mDeviceNameRequest;
        std::shared_ptr<UserAttributes::MyBackupsHandle> mMyBackupsHandleRequest;
        SyncController mSyncController;
        bool mError;
        bool mUserCancelled;
        QStandardItemModel* mFoldersModel;
        ProxyModel* mFoldersProxyModel;
        QMap<QString, BackupInfo> mBackupsStatus;
        QStringList mErrList;
        QWidget* mLoadingWindow;
        Step mCurrentStep;

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
        void onItemChanged(QStandardItem *item = nullptr);
        void onDeviceNameSet(QString deviceName);
        void onMyBackupsFolderHandleSet(mega::MegaHandle h = mega::INVALID_HANDLE);
        void onSyncAddRequestStatus(int errorCode, const QString &errorMsg, const QString &name);
        void onConflictResolved();
};

class WizardDelegate : public QStyledItemDelegate
{
public:
    explicit WizardDelegate(QObject *parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

#endif // BACKUPSWIZARD_H
