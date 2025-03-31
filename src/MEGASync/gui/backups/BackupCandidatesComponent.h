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

    Q_PROPERTY(bool comesFromSettings READ getComesFromSettings NOTIFY comesFromSettingsChanged)
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

    Q_INVOKABLE int rename(const QString& folder, const QString& newName);

    Q_INVOKABLE void remove(const QString& folder);
    Q_INVOKABLE void change(const QString& folder, const QString& newFolder);

    Q_INVOKABLE void selectAllFolders(Qt::CheckState state, bool fromModel);

    Q_INVOKABLE void createBackups(SyncInfo::SyncOrigin syncOrigin);

    void setComesFromSettings(bool value = false);
    bool getComesFromSettings() const;

    Q_INVOKABLE BackupCandidates* getData();

public slots:
    void onBackupsCreationFinished(bool success);

signals:
    void comesFromSettingsChanged(bool value);
    void backupsCreationFinished(bool success);
    void insertFolderAdded(int row);

private:
    bool mComesFromSettings;
    std::shared_ptr<BackupCandidatesController> mBackupCandidatesController;
    QPointer<BackupCandidatesProxyModel> mBackupsProxyModel;
};

#endif // BACKUPS_H
