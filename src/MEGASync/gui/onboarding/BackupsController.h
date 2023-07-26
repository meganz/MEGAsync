#ifndef BACKUPSCONTROLLER_H
#define BACKUPSCONTROLLER_H

#include "QTMegaRequestListener.h"
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

signals:
    void backupFinished(const QString& folder, bool done);
    void backupsCreationFinished(bool success, const QString& message);

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    SyncController* mBackupController;
    int mBackupsToDoSize;
    int mBackupsProcessedWithError;

    // The first field contains the full path and the second contains the backup name
    BackupInfoList mBackupsToDoList;

private slots:
    void onBackupAddRequestStatus(int errorCode, const QString& errorMsg, const QString& name);

};

#endif // BACKUPSCONTROLLER_H
