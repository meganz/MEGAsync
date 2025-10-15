#ifndef ABSTRACTPLATFORM_H
#define ABSTRACTPLATFORM_H

#include "drivedata.h"
#include "MegaApplication.h"
#include "ShellNotifier.h"

#include <QDialog>
#include <QMenu>
#include <QString>
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
    virtual void applyCurrentThemeOnCurrentDialogFrame(QWidget* widget);

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
