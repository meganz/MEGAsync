#ifndef SYNCS_COMPONENT_H
#define SYNCS_COMPONENT_H

#include "QmlDialogWrapper.h"

class SyncsComponent : public QMLComponent
{
    Q_OBJECT

    Q_PROPERTY(bool comesFromSettings READ getComesFromSettings NOTIFY comesFromSettingsChanged)
    Q_PROPERTY(int origin READ getOriginSync NOTIFY originSyncChanged)
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

    void setOriginSync(int value);
    int getOriginSync() const;

    void setRemoteFolder(const QString& remoteFolder);
    QString getRemoteFolder() const;

signals:
    void comesFromSettingsChanged();
    void remoteFolderChanged();
    void originSyncChanged();

private:
    bool mComesFromSettings;
    int mOrigin;
    QString mRemoteFolder;
    SyncInfo::SyncOrigin mSyncOrigin;
};

#endif // SYNCS_COMPONENT_H
