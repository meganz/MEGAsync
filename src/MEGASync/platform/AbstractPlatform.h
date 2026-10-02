#ifndef ABSTRACTPLATFORM_H
#define ABSTRACTPLATFORM_H

#include "drivedata.h"
#include "MegaApplication.h"
#include "ShellNotifier.h"

#include <QDialog>
#include <QMenu>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QWindow>

#include <functional>
#include <string>

struct SelectorInfo
{
    std::function<void(QStringList)> func;
    QWidget* parent;
    QString title;
    QString defaultDir;
    bool multiSelection;
    bool canCreateDirectories;

    SelectorInfo()
        : func(nullptr)
        , parent(nullptr)
        , title(QString())
        , defaultDir(QString())
        , multiSelection(false)
        , canCreateDirectories(false)
    {}
};

class AbstractPlatform: public QObject
{
    Q_OBJECT

public:
    AbstractPlatform() = default;
    virtual ~AbstractPlatform() = default;

    virtual void initialize(int argc, char *argv[]) = 0;
    virtual void prepareForSync();
    virtual bool enableTrayIcon(QString executable);
    virtual void unHideTrayIcon();
    // macOS 27 only; no-ops elsewhere. See adoptStatusItemExpandedInterface().
    virtual void adoptTrayIconExpandedInterface();
    virtual void setTrayPrimaryActionHandler(std::function<void()> handler);
    virtual void notifyItemChange(const QString& localPath, int newState) = 0;
    virtual void notifySyncFileChange(std::string *localPath, int newState) = 0;
    virtual bool startOnStartup(bool value) = 0;
    virtual bool isStartOnStartupActive() = 0;
    virtual bool isTilingWindowManager();
    // Are we running on the Wayland platform? Default uses the live
    // QGuiApplication's platform name (authoritative once the app exists). The
    // Linux override additionally handles the pre-QApplication case (e.g. the
    // scale-factor setup) via environment detection.
    virtual bool isWayland();
    virtual QPoint initialDialogPosition(const QSize& dialogSize) const;
    virtual QPoint initialDialogPosition(const QSize& dialogSize,
                                         const QRect& parentGeometry) const;
    // Moves a plain QWidget dialog to `pos`. On Wayland, move() is ignored by
    // the compositor, so the override uses `visualParent` to set the transient
    // parent instead, letting the compositor centre the dialog itself.
    virtual void moveDialog(QWidget* dialog, const QPoint& pos, QWindow* visualParent = nullptr);
    virtual bool showInFolder(QString pathIn) = 0;
    virtual void startShellDispatcher(MegaApplication *receiver) = 0;
    virtual void stopShellDispatcher() = 0;
    virtual void syncFolderAdded(QString syncPath, QString syncName, QString syncID) = 0;
    virtual void syncFolderRemoved(QString syncPath, QString syncName, QString syncID) = 0;
    virtual void notifyRestartSyncFolders() = 0;
    virtual void notifyAllSyncFoldersAdded() = 0;
    virtual void notifyAllSyncFoldersRemoved() = 0;
    virtual QString preparePathForSync(const QString& path);
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
    virtual bool registerUpdateJob() = 0;
    virtual void uninstall();
    virtual bool isUserActive() = 0;
    virtual QString getDeviceName() = 0;
    virtual void initMenu(QMenu* m, const char* objectName, const bool applyDefaultStyling = true);
    virtual QString getSizeStringLocalizedOSbased(qint64 bytes);
    virtual qint64 getBaseUnitsSize() const;

    virtual void fileSelector(const SelectorInfo& info);
    virtual void folderSelector(const SelectorInfo& info);
    virtual void fileAndFolderSelector(const SelectorInfo& info);
    virtual void raiseFileFolderSelectors();
    virtual void closeFileFolderSelectors(QWidget* parent);

    virtual void addSyncToLeftPane(QString syncPath, QString syncName, QString uuid);
    virtual void removeSyncFromLeftPane(QString syncPath);
    virtual void removeAllSyncsFromLeftPane();

    virtual void disableContextMenu(bool isDisabled) {}

    virtual bool makePubliclyReadable(const QString& fileName);

    // Hides a file or folder from the file manager, keeping its other attributes.
    // On macOS and Linux a leading dot in the name is enough, so the base implementation is a
    // no-op that reports success. Returns false when the item could not be hidden.
    virtual bool setHidden(const QString& path);

    // Creates `folder`, and any missing parent, so that only its owner can access it from
    // the moment it exists: a folder created with the default umask and tightened
    // afterwards is briefly reachable by other users, long enough for one to keep a
    // descriptor on it and reach through it the files created inside later. Parents keep
    // their usual permissions, being shared standard locations. Returns false on failure,
    // describing it in `errorMessage` when one is given.
    //
    // The base implementation only creates the folder, which is enough on Windows: the
    // per-user data location already denies other users through the ACLs it inherits. The
    // POSIX platforms override it to apply the owner-only mode at creation time.
    virtual bool createFolderRestrictedToOwner(const QString& folder, QString* errorMessage);

    // Whether `path` is already inaccessible to users other than its owner. Lets callers
    // report only the folders they actually had to tighten (see restrictToOwner).
    virtual bool isRestrictedToOwner(const QString& path);

    // Restricts a folder so that only its owner can access it, protecting every file
    // inside from other users even though they are created with the default umask. The
    // base implementation applies POSIX owner-only (0700) permissions, which is what
    // macOS and Linux need; Windows overrides it because the per-user data location is
    // already restricted to its owner by the ACLs it inherits.
    virtual bool restrictToOwner(const QString& path);

    virtual void updateDisplayVersionAfterAutoUpdate(int versionCode, bool isPublic);

    virtual void addFileManagerExtensionToSystem() {}

    virtual void enableFileManagerExtension(bool) {}
    virtual bool validateSystemTrayIntegration();

    virtual void calculateInfoDialogCoordinates(const QRect& rect, int *posx, int *posy) = 0;
    virtual void streamWithApp(const QString& app, const QString& url) = 0;
    virtual void processSymLinks() = 0;
    virtual bool loadThemeResource(const QString& theme) = 0;

    std::shared_ptr<AbstractShellNotifier> getShellNotifier();
    virtual DriveSpaceData getDriveData(const QString& path) = 0;

    // AutoUpdate tasks
    virtual void runPostAutoUpdateStep() {}

#if defined(ENABLE_SDK_ISOLATED_GFX)
    virtual QString getGfxProviderPath() = 0;
#endif

    virtual void pinOnTaskbar() {}

    virtual std::string toLocalEncodedPath(const QString& path) const;

    virtual QString getArchUpdateString() const
    {
        return {};
    }

    virtual Preferences::SystemColorScheme getCurrentThemeAppearance() const;
    virtual Preferences::ThemeAppeareance getPanelTheme() const;
    virtual void applyCurrentThemeOnCurrentDialogFrame(QWindow* window);

    // Brings a top-level widget to the foreground even when MEGAsync is not the active
    // process (e.g. a download request triggered from the browser). Default is a no-op;
    // platforms that need OS-specific handling override it.
    virtual void raiseToForeground(QWidget* widget) {}

    virtual void setRenderingBackend() const {}

    // Sets platform-specific environment variables that must be in place before
    // MegaApplication (QApplication) is constructed.
    virtual void setStartupEnvironment() const {}

    // Severs connections with assistive-technology clients before UI teardown.
    // Only needed on Windows; see PlatformImplementation::disconnectAccessibilityClients().
    virtual void disconnectAccessibilityClients() {}

    virtual QStringList getScreenshotsLocation()
    {
        return QStringList();
    }

signals:
    void themeChanged(Preferences::SystemColorScheme theme);

protected:
    std::shared_ptr<AbstractShellNotifier> mShellNotifier = nullptr;

    void logInfoDialogCoordinates(const char *message, const QRect &screenGeometry, const QString &otherInformation);
    QString rectToString(const QRect &rect);
    bool loadRccResources(const QStringList& rccFiles);

    virtual void startThemeMonitor() {}

    virtual void stopThemeMonitor() {}
};

#endif // ABSTRACTPLATFORM_H
