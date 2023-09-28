#ifndef LINUXPLATFORM_H
#define LINUXPLATFORM_H

#include "AbstractPlatform.h"

#include "ExtServer.h"
#include "NotifyServer.h"
#include <xcb/xcb.h>
#include <xcb/xproto.h>


class PlatformImplementation : public AbstractPlatform
{
public:
    PlatformImplementation();

    void initialize(int argc, char *argv[]) override;
    void notifyItemChange(const QString& path, int newState) override;
    void notifySyncFileChange(std::string *localPath, int newState) override;
    bool startOnStartup(bool value) override;
    bool isStartOnStartupActive() override;
    bool isTilingWindowManager() override;
    bool showInFolder(QString pathIn) override;
    void startShellDispatcher(MegaApplication *receiver) override;
    void stopShellDispatcher() override;
    void syncFolderAdded(QString syncPath, QString syncName, QString syncID) override;
    void syncFolderRemoved(QString syncPath, QString syncName, QString syncID) override;
    void notifyRestartSyncFolders() override;
    void notifyAllSyncFoldersAdded() override;
    void notifyAllSyncFoldersRemoved() override;
    QString getDefaultFileBrowserApp() override;
    QString getDefaultOpenApp(QString extension) override;
    QString getDefaultOpenAppByMimeType(QString mimeType) override;
    bool getValue(const char * const name, const bool default_value) override;
    std::string getValue(const char * const name, const std::string &default_value) override;
    QString getWindowManagerName() override;
    bool registerUpdateJob() override;
    bool shouldRunHttpServer() override;
    bool isUserActive() override;
    QString getDeviceName() override;

    void fileSelector(const SelectorInfo& info) override;
    void folderSelector(const SelectorInfo& info) override;
    void fileAndFolderSelector(const SelectorInfo& info) override;

    void calculateInfoDialogCoordinates(const QRect& rect, int* posx, int* posy) override;
    void streamWithApp(const QString& app, const QString& url) override;

private:
    QStringList getListRunningProcesses();
    static xcb_atom_t getAtom(xcb_connection_t * const connection, const char *name);

    ExtServer *ext_server = nullptr;
    NotifyServer *notify_server = nullptr;
    QString autostart_dir;
    QString desktop_file;
    QString set_icon;
    QString custom_icon;
    QString remove_icon;
};

#endif // LINUXPLATFORM_H
