#ifndef SYNCS_COMPONENT_H
#define SYNCS_COMPONENT_H

#include "QmlDialogWrapper.h"
#include "SyncsUtils.h"

class Syncs;
class SyncsData;
class SyncsComponent : public QMLComponent
{
    Q_OBJECT

    Q_PROPERTY(bool comesFromSettings READ getComesFromSettings NOTIFY comesFromSettingsChanged)
    Q_PROPERTY(QString remoteFolder READ getRemoteFolder NOTIFY remoteFolderChanged)
    Q_PROPERTY(SyncsUtils::SyncStatusCode syncStatus READ getSyncStatus WRITE setSyncStatus NOTIFY
                   syncStatusChanged)

public:
    explicit SyncsComponent(QObject* parent = 0);

    QUrl getQmlUrl() override;

    static void registerQmlModules();

    Q_INVOKABLE void openSyncsTabInPreferences() const;
    Q_INVOKABLE void openExclusionsDialog(const QString& folder) const;
    Q_INVOKABLE SyncInfo::SyncOrigin getSyncOrigin() const;

    void setSyncOrigin(SyncInfo::SyncOrigin origin);

    void setComesFromSettings(bool value);
    bool getComesFromSettings() const;

    void setRemoteFolder(const QString& remoteFolder);
    QString getRemoteFolder() const;

public slots:
    void addSync(SyncInfo::SyncOrigin origin,
                 const QString& local,
                 const QString& remote = QLatin1String("/"));
    bool checkLocalSync(const QString& path);
    bool checkRemoteSync(const QString& path);
    void clearRemoteError();
    void clearLocalError();
    void setSyncStatus(SyncsUtils::SyncStatusCode status);
    SyncsUtils::SyncStatusCode getSyncStatus() const;

signals:
    void comesFromSettingsChanged();
    void remoteFolderChanged();
    void originSyncChanged();
    void syncStatusChanged();

private:
    bool mComesFromSettings;
    QString mRemoteFolder;
    SyncInfo::SyncOrigin mSyncOrigin;
    std::unique_ptr<Syncs> mSyncs;
    std::unique_ptr<SyncsData> mSyncsData;
};

#endif // SYNCS_COMPONENT_H
