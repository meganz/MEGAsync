#ifndef BACKUPSCONTROLLER_H
#define BACKUPSCONTROLLER_H

#include "SyncController.h"

#include <mutex>

class BackupsController : public SyncController
{
    Q_OBJECT

public:
    typedef QPair<QString, QString> BackupInfo;
    typedef QList<BackupInfo> BackupInfoList;

    static BackupsController& instance()
    {
        static std::unique_ptr<BackupsController> instance;
        static std::once_flag flag;

        std::call_once(flag, [&]() {
            instance.reset(new BackupsController());
        });

        return *instance;
    }

    BackupsController(const BackupsController&) = delete;
    BackupsController& operator=(const BackupsController&) = delete;

    void addBackups(const BackupInfoList& localPathList);

    QSet<QString> getRemoteFolders() const;

    QString getErrorString(int errorCode, int syncErrorCode) const;

signals:
    void backupFinished(const QString& folder, int errorCode, int syncErrorCode);
    void backupsCreationFinished(bool success);

private:
    BackupsController(QObject *parent = 0);

    mega::MegaApi* mMegaApi;
    int mBackupsToDoSize;
    int mBackupsProcessedWithError;

    // The first field contains the full path and the second contains the backup name
    BackupInfoList mBackupsToDoList;

    bool existsName(const QString& name) const;

private slots:
    void onBackupAddRequestStatus(int errorCode, int syncErrorCode, QString name);

};

#endif // BACKUPSCONTROLLER_H
