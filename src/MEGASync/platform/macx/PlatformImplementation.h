#ifndef MACXPLATFORM_H
#define MACXPLATFORM_H

#include "AbstractPlatform.h"

#include "MacXFunctions.h"
#include "MacXSystemServiceTask.h"
#include "MacXExtServerService.h"

class PlatformImplementation : public AbstractPlatform
{
public:
    PlatformImplementation() = default;

    void initialize(int argc, char *argv[]) override;
    void notifyItemChange(const QString& localPath, int newState) override;
    void notifySyncFileChange(std::string *localPath, int newState) override;
    bool startOnStartup(bool value) override;
    bool isStartOnStartupActive() override;
    bool showInFolder(QString pathIn) override;
    void startShellDispatcher(MegaApplication *receiver) override;
    void stopShellDispatcher() override;
    void syncFolderAdded(QString syncPath, QString syncName, QString syncID) override;
    void syncFolderRemoved(QString syncPath, QString syncName, QString syncID) override;
    void notifyRestartSyncFolders() override;
    void notifyAllSyncFoldersAdded() override;
    void notifyAllSyncFoldersRemoved() override;
    QString getDefaultOpenApp(QString extension) override;
    bool registerUpdateJob() override;
    bool shouldRunHttpServer() override;
    bool shouldRunHttpsServer() override;
    bool isUserActive() override;
    QString getDeviceName() override;
    void initMenu(QMenu* m) override;

    QStringList multipleUpload(QString uploadTitle) override;

    void addFileManagerExtensionToSystem();
    void reloadFileManagerExtension();
    void enableFileManagerExtension(bool value);

private:
    void disableSignalHandler();
    bool isFileManagerExtensionEnabled();

    double getUpTime();

    MacXSystemServiceTask *systemServiceTask = nullptr;
    QPointer<MacXExtServerService> extService = nullptr;
};

#endif // MACXPLATFORM_H
