#ifndef SYNCSCOMPONENT_H
#define SYNCSCOMPONENT_H

#include "qml/QmlDialogWrapper.h"

#include "syncs/control/SyncSettings.h"

class SyncsComponent : public QMLComponent
{
    Q_OBJECT

    Q_PROPERTY(bool comesFromSettings READ getComesFromSettings NOTIFY comesFromSettingsChanged)
    Q_PROPERTY(QString remoteFolder READ getRemoteFolder NOTIFY remoteFolderChanged)

public:
    explicit SyncsComponent(QObject *parent = 0);

    QUrl getQmlUrl() override;
    QString contextName() override;

    static void registerQmlModules();

    Q_INVOKABLE void openSyncsTabInPreferences() const;

    void setComesFromSettings(bool value);
    bool getComesFromSettings() const;

    void setRemoteFolder(const QString& remoteFolder);
    QString getRemoteFolder() const;

signals:
    void comesFromSettingsChanged();
    void remoteFolderChanged();

private:
    bool mComesFromSettings;
    QString mRemoteFolder;
    //mega::MegaHandle mMegaFolderHandle;

    /*
private slots:
    void onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings);
*/


};

#endif // SYNCSCOMPONENT_H
