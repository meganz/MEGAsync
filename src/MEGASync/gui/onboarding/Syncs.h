#ifndef SYNCS_H
#define SYNCS_H

#include "megaapi.h"

#include <QObject>


class SyncController;
class Syncs : public QObject
{
    Q_OBJECT

public:
    Syncs(QObject* parent = nullptr);
    virtual ~Syncs();
    Q_INVOKABLE void addSync(const QString& localPath, mega::MegaHandle remoteHandle = mega::INVALID_HANDLE);

signals:
    void syncSetupSuccess();
    void cantSync(const QString& message, bool localFolderError);
    void cancelSync();

private:
    SyncController* mSyncController;

private slots:
    void onSyncAddRequestStatus(int errorCode, const QString& errorMsg, const QString& name);
};

#endif // SYNCS_H
