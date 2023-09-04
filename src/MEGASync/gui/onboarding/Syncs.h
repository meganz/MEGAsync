#ifndef SYNCS_H
#define SYNCS_H

#include "megaapi.h"
#include "mega/bindings/qt/QTMegaRequestListener.h"
#include "ChooseFolder.h"
#include "syncs/control/SyncController.h"

#include <QObject>

#include <memory>

class SyncController;
class Syncs : public QObject, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    Syncs(QObject* parent = nullptr);
    virtual ~Syncs();
    Q_INVOKABLE void addSync(ChooseLocalFolder* local, ChooseRemoteFolder* remote = nullptr);

    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* e) override;

signals:
    void syncSetupSuccess();
    void cantSync(const QString& message = QString(), bool localFolderError = true);
    void cancelSync();

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    SyncController* mSyncController;
    bool mCreatingDefaultFolder;

    struct SyncProcessInfo
    {
        QString localPath = QString::fromUtf8("");
        SyncController::Syncability syncability = SyncController::CANT_SYNC;
        QString warningMsg = QString::fromUtf8("");
    } mProcessInfo;

    void processRemote(mega::MegaHandle remoteHandle);

private slots:
    void onSyncAddRequestStatus(int errorCode, const QString& errorMsg, const QString& name);
};

#endif // SYNCS_H
