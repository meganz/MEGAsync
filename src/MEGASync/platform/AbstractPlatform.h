#ifndef ABSTRACTPLATFORM_H
#define ABSTRACTPLATFORM_H

#include "MegaApplication.h"
#include "ShellNotifier.h"

#include <QDialog>
#include <QMenu>
#include <QString>
#include <string>

class AbstractPlatform
{
public:
    AbstractPlatform() = default;
    virtual ~AbstractPlatform() = default;

    virtual void initialize(int argc, char *argv[]) = 0;
    virtual void prepareForSync();
    virtual bool enableTrayIcon(QString executable);
    virtual void notifyItemChange(const QString& localPath, int newState) = 0;
    virtual void notifySyncFileChange(std::string *localPath, int newState) = 0;
    virtual bool startOnStartup(bool value) = 0;
    virtual bool isStartOnStartupActive() = 0;
    virtual bool isTilingWindowManager();
    virtual bool showInFolder(QString pathIn) = 0;
    virtual void startShellDispatcher(MegaApplication *receiver) = 0;
    virtual void stopShellDispatcher() = 0;
    virtual void syncFolderAdded(QString syncPath, QString syncName, QString syncID) = 0;
    virtual void syncFolderRemoved(QString syncPath, QString syncName, QString syncID) = 0;
    virtual void notifyRestartSyncFolders() = 0;
    virtual void notifyAllSyncFoldersAdded() = 0;
    virtual void notifyAllSyncFoldersRemoved() = 0;
    virtual QByteArray encrypt(QByteArray data, QByteArray key);
    virtual QByteArray decrypt(QByteArray data, QByteArray key);
    virtual QByteArray getLocalStorageKey();
    virtual QString getDefaultFileBrowserApp();
    virtual QString getDefaultOpenApp(QString extension) = 0;
    virtual QString getDefaultOpenAppByMimeType(QString mimeType);
    virtual bool getValue(const char * const name, const bool default_value);
    virtual std::string getValue(const char * const name, const std::string &default_value);
    virtual QString getWindowManagerName();
    virtual void enableDialogBlur(QDialog *dialog);
    virtual void execBackgroundWindow(QDialog *window);
    virtual void showBackgroundWindow(QDialog *window);
    virtual bool registerUpdateJob() = 0;
    virtual void uninstall();
    virtual bool shouldRunHttpServer() = 0;
    virtual bool shouldRunHttpsServer() = 0;
    virtual bool isUserActive() = 0;
    virtual QString getDeviceName() = 0;
    virtual void initMenu(QMenu* m) = 0;

    virtual void fileSelector(QString title, QString defaultDir, bool multiSelection, QWidget *parent, std::function<void(QStringList)> func) = 0;
    virtual void folderSelector(QString title, QString defaultDir, bool multiSelection, QWidget *parent, std::function<void(QStringList)> func) = 0;
    virtual void fileAndFolderSelector(QString title, QString defaultDir, bool multiSelection, QWidget *parent, std::function<void(QStringList)> func) = 0;
    virtual void raiseFileFolderSelectors() = 0;

    virtual void addSyncToLeftPane(QString syncPath, QString syncName, QString uuid);
    virtual void removeAllSyncsFromLeftPane();
    virtual bool makePubliclyReadable(const QString& fileName);

    virtual void addFileManagerExtensionToSystem() {};
    virtual void reloadFileManagerExtension() {};
    virtual void enableFileManagerExtension(bool) {};

    std::shared_ptr<AbstractShellNotifier> getShellNotifier();

protected:
    std::shared_ptr<AbstractShellNotifier> mShellNotifier = nullptr;
};

#endif // ABSTRACTPLATFORM_H
