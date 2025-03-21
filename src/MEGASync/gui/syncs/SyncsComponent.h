#ifndef SYNCS_COMPONENT_H
#define SYNCS_COMPONENT_H

#include "QmlDialogWrapper.h"

class Syncs;
class SyncsData;
class SyncsComponent : public QMLComponent
{
    Q_OBJECT
    Q_PROPERTY(bool comesFromOnboarding READ getComesFromOnboarding CONSTANT)

public:
    explicit SyncsComponent(QObject* parent = 0);

    QUrl getQmlUrl() override;

    static void registerQmlModules();

    Q_INVOKABLE void openSyncsTabInPreferences() const;
    Q_INVOKABLE void openExclusionsDialog(const QString& folder) const;
    Q_INVOKABLE SyncInfo::SyncOrigin getSyncOrigin() const;
    Q_INVOKABLE bool checkLocalSync(const QString& path);
    Q_INVOKABLE bool checkRemoteSync(const QString& path);
    Q_INVOKABLE void clearRemoteError();
    Q_INVOKABLE void clearLocalError();
    Q_INVOKABLE QString getInitialLocalFolder();
    Q_INVOKABLE QString getInitialRemoteFolder();
    Q_INVOKABLE void chooseRemoteFolderButtonClicked();
    Q_INVOKABLE void chooseLocalFolderButtonClicked();
    Q_INVOKABLE void syncButtonClicked();
    Q_INVOKABLE void setSyncCandidateLocalFolder(const QString& path);
    Q_INVOKABLE void setSyncCandidateRemoteFolder(const QString& path);

    void setSyncOrigin(SyncInfo::SyncOrigin origin);
    void setRemoteFolder(const QString& remoteFolder);
    bool getComesFromOnboarding() const;

private:
    QString mRemoteFolder;
    SyncInfo::SyncOrigin mSyncOrigin;
    std::unique_ptr<Syncs> mSyncs;
    QString mLocalFolderSyncCandidate;
    QString mRemoteFolderSyncCandidate;
};

#endif // SYNCS_COMPONENT_H
