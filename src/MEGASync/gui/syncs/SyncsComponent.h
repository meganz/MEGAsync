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

    Q_INVOKABLE void enteredOnSync();
    Q_INVOKABLE void closingOnboardingDialog();
    Q_INVOKABLE void openExclusionsDialog(const QString& folder) const;
    Q_INVOKABLE void clearRemoteFolderErrorHint();
    Q_INVOKABLE void clearLocalFolderErrorHint();
    Q_INVOKABLE void syncButtonClicked(const QString& localFolder, const QString& megaFolder);
    Q_INVOKABLE void closeDialogButtonClicked();
    Q_INVOKABLE void viewSyncsInSettingsButtonClicked();
    Q_INVOKABLE void exclusionsButtonClicked(const QString& currentPath);
    Q_INVOKABLE void chooseRemoteFolderButtonClicked();
    Q_INVOKABLE void chooseLocalFolderButtonClicked(const QString& currentPath);

    void setSyncOrigin(SyncInfo::SyncOrigin origin);
    void setRemoteFolder(const QString& remoteFolder);
    void setLocalFolder(const QString& localFolder);

signals:
    void remoteFolderChosen(QString remotePath);
    void localFolderChosen(QString folderPath);

private:
    ChooseRemoteFolder mRemoteFolderChooser;
    ChooseLocalFolder mLocalFolderChooser;
    std::unique_ptr<Syncs> mSyncs;
    bool mEnteredOnSyncCreation = false;

    void onRemoteFolderChosen(QString remotePath);
    void onLocalFolderChosen(QString localPath);
    void updateDefaultFolders();
};

#endif // SYNCS_COMPONENT_H
