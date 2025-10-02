#ifndef LINUXPLATFORM_H
#define LINUXPLATFORM_H

#include "AbstractPlatform.h"
#include "ExtServer.h"
#include "NotifyServer.h"

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#ifndef QT_NO_DBUS
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusVariant>
#endif

class PlatformImplementation: public AbstractPlatform
{
    Q_OBJECT

public:
    PlatformImplementation();
    ~PlatformImplementation();

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
    QString preparePathForSync(const QString& path) override;
    QString getDefaultFileBrowserApp() override;
    QString getDefaultOpenApp(QString extension) override;
    QString getDefaultOpenAppByMimeType(QString mimeType) override;
    bool getValue(const char * const name, const bool default_value) override;
    std::string getValue(const char * const name, const std::string &default_value) override;
    QString getWindowManagerName() override;
    bool registerUpdateJob() override;
    bool isUserActive() override;
    QString getDeviceName() override;
    bool validateSystemTrayIntegration() override;

    void fileSelector(const SelectorInfo& info) override;
    void folderSelector(const SelectorInfo& info) override;
    void fileAndFolderSelector(const SelectorInfo& info) override;

    void calculateInfoDialogCoordinates(const QRect& rect, int* posx, int* posy) override;
    void streamWithApp(const QString& app, const QString& url) override;
    void processSymLinks() override;
    bool loadThemeResource(const QString& theme) override;
    DriveSpaceData getDriveData(const QString &path) override;

#if defined(ENABLE_SDK_ISOLATED_GFX)
    QString getGfxProviderPath() override;
#endif

    Preferences::ThemeAppeareance getCurrentThemeAppearance() const override;

private:
    static xcb_atom_t getAtom(xcb_connection_t* const connection, const char* name);
    bool isFedoraWithGnome();
    void promptFedoraGnomeUser();
    bool installAppIndicatorForFedoraGnome();
    int parseDnfOutput(const QString& dnfOutput);
    bool verifyAndEnableAppIndicatorExtension();

    void startThemeMonitor() override;
    void stopThemeMonitor() override;
    static Preferences::ThemeAppeareance themeFromColorSchemeString(const QString& schemeStr);
    static Preferences::ThemeAppeareance themeFromGtkThemeString(const QString& themeStr);
    Preferences::ThemeAppeareance effectiveTheme() const;
    void maybeEmitTheme();
    void setupGSettingsThemeCli();

    ExtServer* ext_server = nullptr;
    NotifyServer *notify_server = nullptr;
    QString autostart_dir;
    QString desktop_file;
    QString custom_icon;
    QProcess mThemeMonitor;

#ifndef QT_NO_DBUS
    void setupSettingsPortalMonitor();
    static Preferences::ThemeAppeareance themeFromDBusVariant(const QDBusVariant& var);
    Preferences::ThemeAppeareance readSettingsPortal();
    QPointer<QDBusInterface> mSettingsPortal;
#endif

    bool mIsSettingsPortalActive = false;
    Preferences::ThemeAppeareance mCurrentPortalTheme =
        Preferences::ThemeAppeareance::UNINITIALIZED;
    bool mUseGtkTheme = false;
    Preferences::ThemeAppeareance mCurrentGSettingsTheme =
        Preferences::ThemeAppeareance::UNINITIALIZED;
    Preferences::ThemeAppeareance mLastEmittedTheme = Preferences::ThemeAppeareance::UNINITIALIZED;

private slots:
    void onGsettingsThemeReadyRead();
#ifndef QT_NO_DBUS
    void onSettingsPortalChanged(const QString& ns, const QString& key, const QDBusVariant& value);
#endif
};

#endif // LINUXPLATFORM_H
