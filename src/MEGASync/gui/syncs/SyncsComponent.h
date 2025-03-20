#ifndef SYNCS_COMPONENT_H
#define SYNCS_COMPONENT_H

#include "QmlDialogWrapper.h"

class Syncs;
class SyncsData;
class SyncsComponent : public QMLComponent
{
    Q_OBJECT

    Q_PROPERTY(bool comesFromSettings READ getComesFromSettings NOTIFY comesFromSettingsChanged)
    Q_PROPERTY(QString remoteFolder READ getRemoteFolder NOTIFY remoteFolderChanged)

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

    Q_INVOKABLE void addSync(SyncInfo::SyncOrigin origin,
                             const QString& local,
                             const QString& remote = QLatin1String("/"));
    Q_INVOKABLE bool checkLocalSync(const QString& path);
    Q_INVOKABLE bool checkRemoteSync(const QString& path);
    Q_INVOKABLE void clearRemoteError();
    Q_INVOKABLE void clearLocalError();
    Q_INVOKABLE QString getInitialLocalFolder();
    Q_INVOKABLE QString getInitialRemoteFolder();

signals:
    void comesFromSettingsChanged();
    void remoteFolderChanged();

private:
    bool mComesFromSettings;
    QString mRemoteFolder;
    SyncInfo::SyncOrigin mSyncOrigin;
    std::unique_ptr<Syncs> mSyncs;
};

#endif // SYNCS_COMPONENT_H
