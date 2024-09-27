#ifndef BACKUPCANDIDATESCONTROLLER_H
#define BACKUPCANDIDATESCONTROLLER_H

#include "BackupCandidates.h"
#include "DataController.h"
#include "SyncInfo.h"

#include <QTimer>

class SyncSettings;

class BackupCandidatesController: public DataController
{
    Q_OBJECT

    static int CHECK_DIRS_TIME;

public:
    BackupCandidatesController();

    void init();

    void calculateFolderSizes();
    void updateSelectedAndTotalSize();
    void setCheckAllState(Qt::CheckState state, bool fromModel = false);
    void setCheckState(int row, bool state);
    QStringList getSelectedCandidates() const;

    void createBackups(int syncOrigin = SyncInfo::SyncOrigin::MAIN_APP_ORIGIN);

    bool setData(int row, const QVariant& value, int role) override;
    bool setData(std::shared_ptr<BackupCandidates::Data> candidate, QVariant value, int role);
    QVariant data(int row, int role) const override;
    QVariant data(std::shared_ptr<BackupCandidates::Data> candidate, int role) const;

    int size() const override;

public slots:
    int insert(const QString& folder);
    void check();
    int rename(const QString& folder, const QString& name);
    void remove(const QString& folder);
    void change(const QString& oldFolder, const QString& newFolder);
    bool checkDirectories();
    void clean(bool resetErrors = false);

signals:
    void backupsCreationFinished(bool succes);

private:
    std::shared_ptr<BackupCandidates> mBackupCandidates;

    void updateModel(QVector<int> roles, std::shared_ptr<BackupCandidates::Data> backupCandidate);
    void updateModel(int role, std::shared_ptr<BackupCandidates::Data> backupCandidate);

    void checkSelectedAll();
    bool isLocalFolderSyncable(const QString& inputPath);
    int selectIfExistsInsertion(const QString& inputPath);
    QList<QList<std::shared_ptr<BackupCandidates::Data>>::const_iterator>
        getRepeatedNameItList(const QString& name);

    bool folderContainsOther(const QString& folder, const QString& other) const;
    bool isRelatedFolder(const QString& folder, const QString& existingPath) const;
    void setAllSelected(bool selected);
    bool checkPermissions(const QString& inputPath);
    void checkDuplicatedBackups(const QStringList& candidateList);
    void reviewConflicts();
    void changeConflictsNotificationText(const QString& text);
    bool existOtherRelatedFolder(const int currentRow);
    bool existsFolder(const QString& inputPath);

    void createConflictsNotificationText(BackupCandidates::BackupErrorCode error);
    QString getSdkErrorString() const;
    QString getSyncErrorString() const;

    std::shared_ptr<BackupCandidates::Data> createData(const QString& folder,
                                                       const QString& displayName,
                                                       bool selected = true);

    bool eventFilter(QObject* watched, QEvent* event) override;

    QTimer mCheckDirsTimer;

private slots:
    void onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings);
    void onBackupsCreationFinished(bool success);
    void onBackupFinished(const QString& folder, int errorCode, int syncErrorCode);
};

#endif // BACKUPCANDIDATESCONTROLLER_H
