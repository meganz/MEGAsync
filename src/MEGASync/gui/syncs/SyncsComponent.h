#ifndef SYNCS_COMPONENT_H
#define SYNCS_COMPONENT_H

#include "ChooseFolder.h"
#include "QmlDialogWrapper.h"

class Syncs;
class SyncsData;
class SyncsComponent : public QMLComponent
{
    Q_OBJECT

public:
    explicit SyncsComponent(QObject* parent = 0);

    QUrl getQmlUrl() override;

    static void registerQmlModules();

    Q_INVOKABLE void openExclusionsDialog(const QString& folder) const;
    Q_INVOKABLE void chooseRemoteFolderButtonClicked();
    Q_INVOKABLE void chooseLocalFolderButtonClicked();
    Q_INVOKABLE void syncButtonClicked();
    Q_INVOKABLE void viewSyncsInSettingsButtonClicked();
    Q_INVOKABLE void exclusionsButtonClicked();

    void setSyncOrigin(SyncInfo::SyncOrigin origin);
    void setRemoteFolder(const QString& remoteFolder);

private:
    std::unique_ptr<Syncs> mSyncs;
    ChooseRemoteFolder mRemoteFolderChooser;
    ChooseLocalFolder mLocalFolderChooser;

    void onRemoteFolderChoosen(QString remotePath);
    void onLocalFolderChoosen(QString localPath);
};

#endif // SYNCS_COMPONENT_H
