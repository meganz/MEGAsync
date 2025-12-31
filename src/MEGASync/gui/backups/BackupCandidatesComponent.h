#ifndef BACKUPS_H
#define BACKUPS_H

#include "BackupCandidatesController.h"
#include "QmlDialogWrapper.h"

class BackupCandidates;
class BackupCandidatesModel;
class BackupCandidatesProxyModel;

class BackupCandidatesComponent: public QMLComponent
{
    Q_OBJECT

    Q_PROPERTY(BackupCandidates* data READ getData CONSTANT)

public:
    explicit BackupCandidatesComponent(QObject* parent = 0);
    ~BackupCandidatesComponent();

    QUrl getQmlUrl() override;

    static void registerQmlModules();

    Q_INVOKABLE void openBackupsTabInPreferences() const;
    Q_INVOKABLE void openExclusionsDialog() const;

    Q_INVOKABLE void confirmFoldersMoveToSelect();
    Q_INVOKABLE void selectFolderMoveToConfirm();
    Q_INVOKABLE void insertFolder(const QString& path);
    void insertFolders(const QStringList& folders);

    Q_INVOKABLE int rename(const QString& folder, const QString& newName);

    Q_INVOKABLE void remove(const QString& folder);
    Q_INVOKABLE void change(const QString& folder, const QString& newFolder);

    Q_INVOKABLE void selectAllFolders(Qt::CheckState state, bool fromModel);

    Q_INVOKABLE void createBackups(SyncInfo::SyncOrigin syncOrigin);

    Q_INVOKABLE void setOrigin(SyncInfo::SyncOrigin origin);

    Q_INVOKABLE BackupCandidates* getData();

public slots:
    void onBackupsCreationFinished(bool success);

signals:
    void backupsCreationFinished(bool success);
    void insertFolderAdded(int row);

private:
    AppStatsEvents::EventType getEventType() const;

private:
    SyncInfo::SyncOrigin mSyncOrigin;
    std::shared_ptr<BackupCandidatesController> mBackupCandidatesController;
    QPointer<BackupCandidatesProxyModel> mBackupsProxyModel;
};

#endif // BACKUPS_H
