#ifndef BACKUPSWIZARD_H
#define BACKUPSWIZARD_H

#include "syncs/control/SyncInfo.h"
#include "syncs/control/SyncController.h"

#include "megaapi.h"

#include <QDialog>
#include <QStringList>
#include <QMap>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QStandardPaths>

namespace Ui
{
    class BackupsWizard;
}
namespace UserAttributes
{
    class DeviceName;
    class MyBackupsHandle;
}
class ProxyModel;

class BackupsWizard : public QDialog
{
        Q_OBJECT

    public:    
        explicit BackupsWizard(QWidget* parent = nullptr);
        ~BackupsWizard();

        static const char* EMPTY_PROPERTY;

    protected:
        void changeEvent(QEvent* event) override;

    private:
        enum Step
        {
            STEP_1 = 0,
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

        void nextStep(const Step& step);
        void setupStep1();
        void setupStep2();
        void handleNameConflicts();
        void setupFinalize();
        void setupBackups();
        void processNextBackupSetup();
        void setupComplete();
        void setupError();

        bool atLeastOneFolderChecked() const;
        void isFolderSyncable(const QString& path, std::function<void (bool)> func, bool displayWarning = false, bool fromCheckAction = false);

        void setupLists();
        void setupHeaders();
        void setupLoadingWindow();
        void showLess();
        void showMore();
        void setCurrentWidgetsSteps(QWidget* widget);
        void updateSize();

        void checkStandardLocations(QList<QStandardPaths::StandardLocation> locations);

        Ui::BackupsWizard* mUi;
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
};

class ProxyModel : public QSortFilterProxyModel
{
    public:
        explicit ProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent), mShowOnlyChecked(false){}

        bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;

        void showOnlyChecked(bool val);
        QModelIndex getIndexByPath(const QString& path) const;

    private:
        bool mShowOnlyChecked;
};

class WizardDelegate : public QStyledItemDelegate
{
public:
    explicit WizardDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent){}

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index);

private:
    QRect calculateIconRect(QStyleOptionViewItem &option) const;
    QRect calculateTextRect(QStyleOptionViewItem &option, QRect iconRect = QRect()) const;

    bool isStep1(const QWidget* view) const;

};

#endif // BACKUPSWIZARD_H
