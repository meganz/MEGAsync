#ifndef SYNCS_COMPONENT_H
#define SYNCS_COMPONENT_H

#include "QmlDialogWrapper.h"

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
};

#endif // SYNCS_COMPONENT_H
