#ifndef SYNCS_H
#define SYNCS_H

#include "megaapi.h"
#include "mega/bindings/qt/QTMegaRequestListener.h"
#include "syncs/control/SyncController.h"

#include <QObject>

#include <memory>

class SyncController;
class Syncs : public QObject, public mega::MegaRequestListener
{
    Q_OBJECT

    Q_PROPERTY(QString DEFAULT_MEGA_FOLDER READ getDefaultMegaFolder CONSTANT FINAL)
    Q_PROPERTY(QString DEFAULT_MEGA_PATH READ getDefaultMegaPath CONSTANT FINAL)

public:
    Syncs(QObject* parent = nullptr);
    virtual ~Syncs() = default;
    Q_INVOKABLE void addSync(const QString& local, const QString& remote = QLatin1String("/"));
    Q_INVOKABLE bool checkLocalSync(const QString& path) const;
    Q_INVOKABLE bool checkRemoteSync(const QString& path) const;
    Q_INVOKABLE QString getDefaultMegaFolder() const;
    Q_INVOKABLE QString getDefaultMegaPath() const;

    static const QString DEFAULT_MEGA_FOLDER;
    static const QString DEFAULT_MEGA_PATH;

signals:
    void syncSetupSuccess();
    void cantSync(const QString& message = QString(), bool localFolderError = true);

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<SyncController> mSyncController;
    QString mRemoteFolder;
    QString mLocalFolder;
    bool mCreatingFolder;

    bool errorOnSyncPaths(const QString& localPath, const QString& remotePath);
    bool helperCheckLocalSync(const QString& path, QString& errorMessage) const;
    bool helperCheckRemoteSync(const QString& path, QString& errorMessage) const;

private slots:
    void onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString errorMsg, QString name);
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* e) override;
};

#endif // SYNCS_H
