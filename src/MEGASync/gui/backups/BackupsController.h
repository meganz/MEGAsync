#ifndef BACKUPSCONTROLLER_H
#define BACKUPSCONTROLLER_H

#include "syncs/control/SyncController.h"

class BackupsController : public QObject
{
    Q_OBJECT

public:
    typedef QPair<QString, QString> BackupInfo;
    typedef QList<BackupInfo> BackupInfoList;

    BackupsController(QObject *parent = 0);

    void addBackups(const BackupInfoList& localPathList);

    QSet<QString> getRemoteFolders() const;

    static QString getErrorString(int errorCode, int syncErrorCode);

signals:
    void backupFinished(const QString& folder, int errorCode, int syncErrorCode);
    void backupsCreationFinished(bool success);

private:
    mega::MegaApi* mMegaApi;
    SyncController* mBackupController;
    int mBackupsToDoSize;
    int mBackupsProcessedWithError;

    // The first field contains the full path and the second contains the backup name
    BackupInfoList mBackupsToDoList;

    bool existsName(const QString& name) const;

private slots:
    void onBackupAddRequestStatus(int errorCode, int syncErrorCode, QString name);

};

#endif // BACKUPSCONTROLLER_H
